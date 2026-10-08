/* *****************************************************************************
 * @file        lora.cpp
 * @brief       SX1278 LoRa transport for ESP32 WROOM and host simulation.
 * @author      OTAValveMaster Contributors
 * @date        2026-09-14
 *
 * @license     Project License
 *              Copyright (c) 2026 OTAValveMaster Contributors
 *              All rights reserved.
 ******************************************************************************/

#include "lora.h"

#include <stdlib.h>
#include <string.h>
#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

ERROR_CODE initialize_LoRa(void)
{
    SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
    LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
    LoRa.setSPIFrequency(LORA_SPI_FREQUENCY_HZ);

    if (!LoRa.begin(LORA_FREQUENCY_HZ)) {
        print_err("%s:SX1278 initialization failed", __FUNCTION__);
        return ERR_NET_IF_FAIL;
    }

    LoRa.setTxPower(LORA_TX_POWER_DBM);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_SIGNAL_BANDWIDTH_HZ);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.enableCrc();
    LoRa.receive();

    print_info("%s:SX1278 ready at %u Hz", __FUNCTION__, LORA_FREQUENCY_HZ);
    return ERR_OK;
}

ERROR_CODE send_LoRa(void *pvBuffer)
{
    const size_t payload_length = sizeof(SLAVE_ORDER_BUFFER);

    if (pvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (LoRa.beginPacket() == 0) {
        return ERR_NET_IF_FAIL;
    }

    if (LoRa.write((const uint8_t *)pvBuffer, payload_length) != payload_length) {
        LoRa.endPacket();
        LoRa.receive();
        return ERR_NET_IF_FAIL;
    }

    if (LoRa.endPacket() != 1) {
        LoRa.receive();
        return ERR_NET_IF_FAIL;
    }

    LoRa.receive();
    return ERR_OK;
}

ERROR_CODE receive_LoRa(void **ppvBuffer)
{
    int packet_size;
    unsigned long start_time;
    size_t bytes_read = 0U;
    uint8_t *buffer;

    if (ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    *ppvBuffer = NULL;
    start_time = millis();
    do {
        packet_size = LoRa.parsePacket();
        if (packet_size != 0) {
            break;
        }
        delay(1U);
    } while ((millis() - start_time) < LORA_RX_TIMEOUT_MS);

    if (packet_size == 0) {
        return ERR_TIMEOUT;
    }
    if (packet_size != (int)sizeof(ACK_BUFFER)) {
        while (LoRa.available()) {
            (void)LoRa.read();
        }
        return ERR_INVALID_REQUEST;
    }

    buffer = (uint8_t *)malloc(sizeof(ACK_BUFFER));
    if (buffer == NULL) {
        return ERR_MEMORY_ALLOCATION_FAIL;
    }

    while (LoRa.available() && bytes_read < sizeof(ACK_BUFFER)) {
        int value = LoRa.read();
        if (value < 0) {
            free(buffer);
            return ERR_NET_IF_FAIL;
        }
        buffer[bytes_read++] = (uint8_t)value;
    }

    if (bytes_read != sizeof(ACK_BUFFER)) {
        free(buffer);
        return ERR_INTEGRITY_CHECK_FAILED;
    }

    *ppvBuffer = buffer;
    return ERR_OK;
}
