#ifndef IPC_H
#define IPC_H

#include <stddef.h>
#include "gen.h"

ERROR_CODE initialize_ipc(void);
int receive_ipc_from_user(void **ppvBuffer);
int receive_ipc_from_slave(void **ppvBuffer);
int send_ipc_to_user(const void *pvBuffer);
int send_ipc_to_slave(const void *pvBuffer);

#endif /* IPC_H */
