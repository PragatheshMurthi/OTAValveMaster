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
#ifndef LORA_H
#define LORA_H

#include "comm.h"

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

#endif /* LORA_H */
