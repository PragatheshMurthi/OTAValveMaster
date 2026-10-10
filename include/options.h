/* *****************************************************************************
 * @file        options.h
 * @brief       Compile-time feature flags, limits, and transport settings.
 * @author      OTAValveMaster Contributors
 * @date        2026-09-14
 *
 * @license     Project License
 *              Copyright (c) 2026 OTAValveMaster Contributors
 *              All rights reserved.
 ******************************************************************************/

/*============================================================================*/
/*                                  INCLUDES                                  */
/*============================================================================*/
#ifndef OPTIONS_H
#define OPTIONS_H

/*============================================================================*/
/*                         DEFINES & MACROS & ENUMS                           */
/*============================================================================*/

#define MAX_ORDER_RESULT_LENGTH 32
#define MAX_USR_ACK_RESULT_LENGTH 128
#define MAX_ACK_WAIT_TIME 5000U
#define MAX_USER_REQUESTS 16U
#define MAX_VALVE_ID 128U
#define MIN_TIMER_DURATION 1U
#define MAX_TIMER_DURATION 3600U

#define ENABLE_LORA_FOR_LAN

//#define ENABLE_WIFI_FOR_WAN
#define ENABLE_GSM_FOR_WAN

//#define OTA_HAL_SIMULATION 1


//#define ENABLE_IPC_FOR_LAN_SIM
//#define ENABLE_IPC_FOR_WAN_SIM

/* SX1278 and ESP32 WROOM hardware configuration. */
#define LORA_SCK_PIN 18
#define LORA_MISO_PIN 19
#define LORA_MOSI_PIN 23
#define LORA_SS_PIN 5
#define LORA_RESET_PIN 14
#define LORA_DIO0_PIN 26
#define LORA_SPI_FREQUENCY_HZ 8000000U
#define LORA_FREQUENCY_HZ 433000000U
#define LORA_TX_POWER_DBM 17U
#define LORA_SPREADING_FACTOR 7U
#define LORA_SIGNAL_BANDWIDTH_HZ 125000L
#define LORA_CODING_RATE 5
#define LORA_PREAMBLE_LENGTH 8U
#define LORA_SYNC_WORD 0x34U
#define LORA_RX_TIMEOUT_MS MAX_ACK_WAIT_TIME

/* SIM800L UART, power, and SMS configuration. */
#define GSM_UART_PORT 2
#define UART_SCLK_DEFAULT 
#define GSM_SERIAL_BAUD 115200U
#define GSM_TX_PIN 17
#define GSM_RX_PIN 16
#define GSM_PWRKEY_PIN 4
#define GSM_PWRKEY_ACTIVE_LOW 1
#define GSM_SMS_TARGET_NUMBER "+10000000000"
#define GSM_AT_TIMEOUT_MS 5000U
#define GSM_NETWORK_TIMEOUT_MS 30000U
#define GSM_SMS_POLL_INTERVAL_MS 1000U
#define GSM_SMS_RESPONSE_SIZE 512U
#define GSM_SMS_TEXT_SIZE 160U

#define ENABLE_DBG
#define ENABLE_INFO
#define ENABLE_ERROR
#define ENABLE_UART_LOG

#endif /* OPTIONS_H */
