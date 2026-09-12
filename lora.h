#ifndef LORA_H
#define LORA_H

#include "comm.h"

ERROR_CODE initialize_LoRa(void);
ERROR_CODE receive_LoRa(void **ppvBuffer);
ERROR_CODE send_LoRa(void *pvBuffer);

#endif /* LORA_H */
