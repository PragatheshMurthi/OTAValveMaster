/* *****************************************************************************
 * @file        options.h
 * @brief       Compile-time feature flags, limits, and transport settings.
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
#ifndef OPTIONS_H
#define OPTIONS_H

/*============================================================================*/
/*                         DEFINES & MACROS & ENUMS                           */
/*============================================================================*/

#define MAX_ORDER_RESULT_LENGTH 32
#define MAX_USR_ACK_RESULT_LENGTH 128
#define MAX_ACK_WAIT_TIME 5000U
#define MAX_USER_REQUESTS 16U
#define MAX_VALVE_ID 128U
#define MIN_TIMER_DURATION 1U
#define MAX_TIMER_DURATION 3600U

//#define ENABLE_LORA_FOR_LAN
#define ENABLE_IPC_FOR_LAN_SIM

//#define ENABLE_WIFI_FOR_WAN
//#define ENABLE_GSM_FOR_WAN
#define ENABLE_IPC_FOR_WAN_SIM

#define OTA_HAL_SIMULATION 1
/* #define OTA_HAL_REAL_TARGET 1 */

#define ENABLE_DBG
#define ENABLE_INFO
#define ENABLE_ERROR
#define ENABLE_UART_LOG

#endif /* OPTIONS_H */
