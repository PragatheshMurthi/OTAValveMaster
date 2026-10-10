/* *****************************************************************************
 * @file        lora.h
 * @brief       Public LoRa transport initialization and I/O declarations.
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
#ifndef OTA_VALVE_LORAINT_H
#define OTA_VALVE_LORAINT_H

#include "comm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                            FUNCTION PROTOTYPES                             */
/*============================================================================*/

/** Initializes the LoRa interface. */
ERROR_CODE initialize_LoRa(void);

/** Receives an allocated acknowledgment buffer from LoRa.
 *  @param[out] ppvBuffer Receives the allocated buffer.
 *  @return ERR_OK on success, otherwise a transport error.
 */
ERROR_CODE receive_LoRa(void **ppvBuffer);

/** Sends a buffer through LoRa.
 *  @param[in] pvBuffer Buffer to send.
 *  @return ERR_OK on success, otherwise a transport error.
 */
ERROR_CODE send_LoRa(void *pvBuffer);

#ifdef __cplusplus
}
#endif

#endif /* OTA_VALVE_LORAINT_H */
