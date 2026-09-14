/* *****************************************************************************
 * @file        netintr.h
 * @brief       Public network hardware abstraction interfaces and frames.
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
#ifndef __NETINTR__
#define __NETINTR__

#include <stddef.h>

#include "gen.h"

/*============================================================================*/
/*                            TYPE DEFINITIONS                                */
/*============================================================================*/

/**
 * @typedef HAL_FRAME
 * @brief Generic frame exchanged through a network hardware abstraction.
 */
typedef struct {
    UINT32 u32Length;                   /**< Payload and frame length in bytes. */
    UINT32 u32CRC;                      /**< CRC of the frame data. */
    UINT8 u8PayloadType;                /**< Encoded payload type. */
    UINT8 u8Source;                     /**< Source endpoint identifier. */
    UINT8 u8Target;                     /**< Target endpoint identifier. */
    UINT8 u8Flags;                      /**< Frame control flags. */
    UINT8 acPayload[512U];              /**< Frame payload storage. */
} HAL_FRAME;

/** Initializes all enabled network interfaces. */
int initializeInterfaces(void);

/** Initializes the LoRa interface. */
int initializeLoRa(void);

/** Initializes the Wi-Fi interface. */
int initializeWiFi(void);

/** Initializes the GSM interface. */
int initializeGSM(void);

/** Sends one payload through the configured hardware abstraction.
 *  @param[in] pvPayload Payload to send.
 *  @param[in] uPayloadLength Payload length in bytes.
 *  @return Zero on success, otherwise an interface error.
 */
int hal_send(const void *pvPayload, size_t uPayloadLength);

/** Receives one payload from the configured hardware abstraction.
 *  @param[out] ppvBuffer Receives an allocated payload buffer.
 *  @return Zero on success, otherwise an interface error.
 */
int hal_receive(void **ppvBuffer);

/** Receives one payload with a timeout.
 *  @param[out] pvBuffer Destination buffer.
 *  @param[in] u32TimeoutMs Timeout in milliseconds.
 *  @return Zero on success, otherwise an interface error.
 */
int hal_receive_timeout(void *pvBuffer, UINT32 u32TimeoutMs);

#endif /* __NETINTR__ */