/* *****************************************************************************
 * @file        netintr.c
 * @brief       Hardware abstraction layer routing for network interfaces.
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
#include "netintr.h"
#include "lora.h"
#include "gsm.h"

#include <time.h>

/*============================================================================*/
/*                         DEFINES & MACROS & ENUMS                           */
/*============================================================================*/

/*============================================================================*/
/*                            FUNCTION PROTOTYPES                             */
/*============================================================================*/

/*============================================================================*/
/*                          GLOBAL / STATIC VARIABLES                         */
/*============================================================================*/

/*============================================================================*/
/*                           FUNCTION DEFINITIONS                             */
/*============================================================================*/

static UINT32 hal_crc32(const void *pvData, size_t uLength)
{
    const UINT8 *pucData = (const UINT8 *)pvData;
    UINT32 u32Crc = 0xFFFFFFFFu;

    if (pvData == NULL || uLength == 0U) {
        return 0U;
    }

    for (size_t i = 0U; i < uLength; ++i) {
        u32Crc ^= (UINT32)pucData[i];
        for (UINT8 bit = 0U; bit < 8U; ++bit) {
            if ((u32Crc & 1U) != 0U) {
                u32Crc = (u32Crc >> 1U) ^ 0xEDB88320u;
            } else {
                u32Crc >>= 1U;
            }
        }
    }

    return (~u32Crc);
}

int initializeInterfaces(void)
{

#ifdef ENABLE_LORA_FOR_LAN
    if (initialize_LoRa() != ERR_OK) {
        return ERR_NET_IF_FAIL;
    }
#endif

#ifdef ENABLE_GSM_FOR_WAN
    if (initialize_GSM() != ERR_OK) {
        return ERR_NET_IF_FAIL;
    }
#endif

#ifdef ENABLE_IPC_FOR_LAN_SIM
    if (initialize_ipc() != ERR_OK) {
        return ERR_NET_IF_FAIL;
    }
#endif
    return ERR_OK;
}

int hal_receive_from_user(void **ppvBuffer)
{
    if (ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    *ppvBuffer = NULL;
#ifdef ENABLE_GSM_FOR_WAN    
    receive_from_gsm(ppvBuffer);
#endif

#ifdef ENABLE_IPC_FOR_WAN_SIM    
    receive_ipc_from_user(ppvBuffer);
#endif
    return ERR_OK;
}

int hal_send_to_user(void *pvBuffer)
{
    if (pvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

#ifdef ENABLE_GSM_FOR_WAN    
    send_gsm(pvBuffer);
#endif

#ifdef ENABLE_IPC_FOR_WAN_SIM    
    send_ipc_to_user(pvBuffer);
#endif
    return ERR_OK;
}

int hal_send_to_slave(void *pvBuffer)
{
    if (pvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

#ifdef ENABLE_LORA_FOR_LAN  
    send_LoRa(pvBuffer);
#endif

#ifdef ENABLE_IPC_FOR_LAN_SIM
    send_ipc_to_slave(pvBuffer);
#endif
    return ERR_OK;
}

int hal_receive_from_slave(void **ppvBuffer)
{
    if (ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    *ppvBuffer = NULL;
#ifdef ENABLE_LORA_FOR_LAN   
    receive_LoRa(ppvBuffer);
#endif

#ifdef ENABLE_IPC_FOR_LAN_SIM
    receive_ipc_from_slave(ppvBuffer);
#endif
    return ERR_OK;
}


