/* *****************************************************************************
 * @file        gsm.c
 * @brief       SIM800L SMS transport using UART AT commands.
 * @author      OTAValveMaster Contributors
 * @date        2026-09-14
 *
 * @license     Project License
 *              Copyright (c) 2026 OTAValveMaster Contributors
 *              All rights reserved.
 ******************************************************************************/

#include "gsm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define GSM_UART ((uart_port_t)GSM_UART_PORT)
#define GSM_CTRL_Z 0x1AU

static void gsm_delay_ms(uint32_t delay_ms)
{
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

static void flush_gsm_input(void)
{
    uart_flush_input(GSM_UART);
}

static int response_has_terminal_status(const char *response)
{
    return strstr(response, "OK") != NULL ||
           strstr(response, "ERROR") != NULL ||
           strstr(response, ">") != NULL;
}

static int read_gsm_response(char *response, size_t response_size,
                             uint32_t timeout_ms)
{
    size_t response_length = 0U;
    uint32_t elapsed_ms = 0U;
    uint8_t byte;

    if (response == NULL || response_size < 2U) {
        return 0;
    }

    response[0] = '\0';
    while (elapsed_ms < timeout_ms && response_length < response_size - 1U) {
        int read_count = uart_read_bytes(GSM_UART, &byte, 1U,
                                         pdMS_TO_TICKS(1U));
        if (read_count == 1) {
            response[response_length++] = (char)byte;
            response[response_length] = '\0';
            if (response_has_terminal_status(response)) {
                return strstr(response, "ERROR") == NULL;
            }
        } else {
            elapsed_ms++;
        }
    }

    return response_has_terminal_status(response) &&
           strstr(response, "ERROR") == NULL;
}

static int send_at_command(const char *command, char *response,
                           size_t response_size, uint32_t timeout_ms)
{
    if (command == NULL) {
        return 0;
    }

    flush_gsm_input();
    uart_write_bytes(GSM_UART, command, strlen(command));
    uart_write_bytes(GSM_UART, "\r", 1U);
    return read_gsm_response(response, response_size, timeout_ms);
}

static int wait_for_network(void)
{
    char response[GSM_SMS_RESPONSE_SIZE];
    uint32_t elapsed_ms = 0U;

    while (elapsed_ms < GSM_NETWORK_TIMEOUT_MS) {
        if (send_at_command("AT+CREG?", response, sizeof(response),
                            GSM_AT_TIMEOUT_MS) &&
            (strstr(response, ",1") != NULL || strstr(response, ",5") != NULL)) {
            return 1;
        }
        gsm_delay_ms(1000U);
        elapsed_ms += 1000U;
    }

    return 0;
}

static void trim_line(char *text)
{
    size_t length;
    char *start;

    if (text == NULL) {
        return;
    }

    start = text;
    while (*start == '\r' || *start == '\n' || *start == ' ') {
        start++;
    }
    if (start != text) {
        memmove(text, start, strlen(start) + 1U);
    }

    length = strlen(text);
    while (length > 0U && (text[length - 1U] == '\r' ||
                           text[length - 1U] == '\n' ||
                           text[length - 1U] == ' ')) {
        text[--length] = '\0';
    }
}

static int text_equals(const char *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        char left_char = *left++;
        char right_char = *right++;

        if (left_char >= 'a' && left_char <= 'z') {
            left_char = (char)(left_char - ('a' - 'A'));
        }
        if (right_char >= 'a' && right_char <= 'z') {
            right_char = (char)(right_char - ('a' - 'A'));
        }
        if (left_char != right_char) {
            return 0;
        }
    }

    return *left == '\0' && *right == '\0';
}

static ERROR_CODE sms_to_request(const char *sms_text, void **ppvBuffer)
{
    char action[24];
    unsigned int valve_id = 0U;
    unsigned int timer_duration = 0U;
    UREQ_BUFFER *request;
    UREQ_METADATA *metadata;
    const size_t request_length = sizeof(UREQ_METADATA) + sizeof(UREQ_BUFFER);
    UINT32 request_type;

    if (sms_text == NULL || ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (sscanf(sms_text, "%23s %u %u", action, &valve_id, &timer_duration) < 2 ||
        valve_id == 0U || valve_id > MAX_VALVE_ID) {
        return ERR_INVALID_REQUEST;
    }

    if (text_equals(action, "OPEN")) {
        request_type = REQUEST_VALVE_OPEN;
    } else if (text_equals(action, "CLOSE")) {
        request_type = REQUEST_VALVE_CLOSE;
    } else if (text_equals(action, "STATUS")) {
        request_type = REQUEST_VALVE_STATUS;
    } else if (text_equals(action, "OPEN_TIMED")) {
        request_type = REQUEST_VALVE_OPEN_TIMED_CLOSE;
    } else if (text_equals(action, "CLOSE_TIMED")) {
        request_type = REQUEST_VALVE_CLOSE_TIMED_OPEN;
    } else {
        return ERR_INVALID_REQUEST;
    }

    if ((request_type == REQUEST_VALVE_OPEN_TIMED_CLOSE ||
         request_type == REQUEST_VALVE_CLOSE_TIMED_OPEN) &&
        (timer_duration < MIN_TIMER_DURATION || timer_duration > MAX_TIMER_DURATION)) {
        return ERR_INVALID_REQUEST;
    }

    *ppvBuffer = malloc(request_length);
    if (*ppvBuffer == NULL) {
        return ERR_MEMORY_ALLOCATION_FAIL;
    }

    memset(*ppvBuffer, 0, request_length);
    metadata = (UREQ_METADATA *)*ppvBuffer;
    request = (UREQ_BUFFER *)((uint8_t *)*ppvBuffer + sizeof(UREQ_METADATA));
    metadata->u32RequestLength = (UINT32)request_length;
    metadata->u32NumberOfRequests = 1U;
    metadata->u32RequestSequenceNumber = (UINT32)xTaskGetTickCount();
    request->u32ValveID = valve_id;
    request->u8RequestType = request_type;
    request->u32TimerDuration = timer_duration;
    metadata->u32RequestCRC = calculate_crc(*ppvBuffer, (UINT16)request_length);
    return ERR_OK_INT;
}

ERROR_CODE initialize_GSM(void)
{
    uart_config_t uart_config = {
        .baud_rate = GSM_SERIAL_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };

    char response[GSM_SMS_RESPONSE_SIZE];
    int inactive_level = GSM_PWRKEY_ACTIVE_LOW ? 1 : 0;
    int active_level = GSM_PWRKEY_ACTIVE_LOW ? 0 : 1;

    if (uart_driver_install(GSM_UART, 2048U, 0U, 0U, NULL, 0U) != ESP_OK ||
        uart_param_config(GSM_UART, &uart_config) != ESP_OK ||
        uart_set_pin(GSM_UART, GSM_TX_PIN, GSM_RX_PIN,
                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        return ERR_NET_IF_FAIL;
    }

    gpio_set_direction(GSM_PWRKEY_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(GSM_PWRKEY_PIN, inactive_level);
    gsm_delay_ms(100U);
    gpio_set_level(GSM_PWRKEY_PIN, active_level);
    gsm_delay_ms(1100U);
    gpio_set_level(GSM_PWRKEY_PIN, inactive_level);
    gsm_delay_ms(3000U);

    if (!send_at_command("AT", response, sizeof(response), GSM_AT_TIMEOUT_MS) ||
        !send_at_command("ATE0", response, sizeof(response), GSM_AT_TIMEOUT_MS) ||
        !send_at_command("AT+CPIN?", response, sizeof(response), GSM_AT_TIMEOUT_MS) ||
        strstr(response, "READY") == NULL ||
        !send_at_command("AT+CMGF=1", response, sizeof(response), GSM_AT_TIMEOUT_MS) ||
        !send_at_command("AT+CSCS=\"GSM\"", response, sizeof(response), GSM_AT_TIMEOUT_MS) ||
        !send_at_command("AT+CNMI=0,0,0,0,0", response, sizeof(response), GSM_AT_TIMEOUT_MS) ||
        !wait_for_network()) {
        return ERR_NET_IF_FAIL;
    }

    return ERR_OK_INT;
}

ERROR_CODE receive_from_gsm(void **ppvBuffer)
{
    char response[GSM_SMS_RESPONSE_SIZE];
    char *header;
    char *message;
    char *message_end;
    unsigned int message_index = 0U;
    char delete_command[32];

    if (ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }
    *ppvBuffer = NULL;

    if (!send_at_command("AT+CMGL=\"REC UNREAD\"", response, sizeof(response),
                         GSM_AT_TIMEOUT_MS)) {
        return ERR_TIMEOUT_INT;
    }

    header = strstr(response, "+CMGL:");
    if (header == NULL) {
        return ERR_TIMEOUT_INT;
    }
    if (sscanf(header, "+CMGL: %u", &message_index) != 1) {
        return ERR_INVALID_REQUEST;
    }

    message = strchr(header, '\n');
    if (message == NULL) {
        return ERR_INVALID_REQUEST;
    }
    message++;
    message_end = strchr(message, '\r');
    if (message_end == NULL) {
        message_end = strchr(message, '\n');
    }
    if (message_end != NULL) {
        *message_end = '\0';
    }
    trim_line(message);

    snprintf(delete_command, sizeof(delete_command), "AT+CMGD=%u", message_index);
    (void)send_at_command(delete_command, response, sizeof(response), GSM_AT_TIMEOUT_MS);
    return sms_to_request(message, ppvBuffer);
}

ERROR_CODE send_gsm(void *pvBuffer)
{
    char response[GSM_SMS_RESPONSE_SIZE];
    char message[MAX_USR_ACK_RESULT_LENGTH];
    size_t message_length;
    char command[64];

    if (pvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    snprintf(message, sizeof(message), "SEQ:%u STATUS:%s",
             ((USR_ACK_BUFFER *)pvBuffer)->u32SequenceNumber,
             ((USR_ACK_BUFFER *)pvBuffer)->acRequestStatus);
    message_length = strlen(message);

    flush_gsm_input();
    snprintf(command, sizeof(command), "AT+CMGS=\"%s\"\r", GSM_SMS_TARGET_NUMBER);
    uart_write_bytes(GSM_UART, command, strlen(command));
    if (!read_gsm_response(response, sizeof(response), GSM_AT_TIMEOUT_MS) ||
        strstr(response, ">") == NULL) {
        return ERR_NET_IF_FAIL;
    }

    uart_write_bytes(GSM_UART, message, message_length);
    uart_write_bytes(GSM_UART, (const char *)&(uint8_t){GSM_CTRL_Z}, 1U);
    if (!read_gsm_response(response, sizeof(response), GSM_AT_TIMEOUT_MS * 2U)) {
        return ERR_NET_IF_FAIL;
    }

    return ERR_OK_INT;
}