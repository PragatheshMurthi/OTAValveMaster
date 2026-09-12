#ifndef GSM_H
#define GSM_H

#include "comm.h"

ERROR_CODE initialize_GSM(void);
ERROR_CODE receive_from_gsm(void **ppvBuffer);
ERROR_CODE send_gsm(void *pvBuffer);

#endif /* GSM_H */
