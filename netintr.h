#ifndef __NETINTR__
#define __NETINTR__

#include <stddef.h>

#include "gen.h"

typedef struct {
    UINT32 u32Length;
    UINT32 u32CRC;
    UINT8 u8PayloadType;
    UINT8 u8Source;
    UINT8 u8Target;
    UINT8 u8Flags;
    UINT8 acPayload[512U];
} HAL_FRAME;

int initializeInterfaces(void);
int initializeLoRa(void);
int initializeWiFi(void);
int initializeGSM(void);
int hal_send(const void *pvPayload, size_t uPayloadLength);
int hal_receive(void **ppvBuffer);
int hal_receive_timeout(void *pvBuffer, UINT32 u32TimeoutMs);

#endif /* __NETINTR__ */