/* *****************************************************************************
 * @file        ipc.h
 * @brief       Public UDP inter-process communication declarations.
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
#ifndef IPC_H
#define IPC_H

#include <stddef.h>
#include "gen.h"

/*============================================================================*/
/*                            FUNCTION PROTOTYPES                             */
/*============================================================================*/

/** Initializes the master IPC sockets. */
ERROR_CODE initialize_ipc(void);

/** Receives an allocated request buffer from the user IPC socket.
 *  @param[out] ppvBuffer Receives the allocated buffer.
 *  @return ERR_OK on success, otherwise an IPC error.
 */
int receive_ipc_from_user(void **ppvBuffer);

/** Receives an allocated acknowledgment from the slave IPC socket.
 *  @param[out] ppvBuffer Receives the allocated buffer.
 *  @return ERR_OK on success, otherwise an IPC error.
 */
int receive_ipc_from_slave(void **ppvBuffer);

/** Sends a buffer to the user IPC endpoint.
 *  @param[in] pvBuffer Buffer to send.
 *  @return ERR_OK on success, otherwise an IPC error.
 */
int send_ipc_to_user(const void *pvBuffer);

/** Sends a buffer to the slave IPC endpoint.
 *  @param[in] pvBuffer Buffer to send.
 *  @return ERR_OK on success, otherwise an IPC error.
 */
int send_ipc_to_slave(const void *pvBuffer);

#endif /* IPC_H */
