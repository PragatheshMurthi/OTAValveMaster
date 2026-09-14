/* *****************************************************************************
 * @file        gsm.h
 * @brief       Public GSM transport initialization and I/O declarations.
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
#ifndef GSM_H
#define GSM_H

#include "comm.h"

/*============================================================================*/
/*                            FUNCTION PROTOTYPES                             */
/*============================================================================*/

/** Initializes the GSM interface. */
ERROR_CODE initialize_GSM(void);

/** Receives an allocated request buffer from GSM.
 *  @param[out] ppvBuffer Receives the allocated buffer.
 *  @return ERR_OK on success, otherwise a transport error.
 */
ERROR_CODE receive_from_gsm(void **ppvBuffer);

/** Sends a buffer through GSM.
 *  @param[in] pvBuffer Buffer to send.
 *  @return ERR_OK on success, otherwise a transport error.
 */
ERROR_CODE send_gsm(void *pvBuffer);

#endif /* GSM_H */
