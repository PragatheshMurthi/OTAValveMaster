/* *****************************************************************************
 * @file        main.c
 * @brief       Master application entry points and request-processing loop.
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
#include <stdbool.h>

#include "comm.h"
#include "motor.h"
#include "netintr.h"

/*============================================================================*/
/*                         DEFINES & MACROS & ENUMS                           */
/*============================================================================*/

/*============================================================================*/
/*                            FUNCTION PROTOTYPES                             */
/*============================================================================*/

/*============================================================================*/
/*                          GLOBAL / STATIC VARIABLES                         */
/*============================================================================*/

void startWorking(void);

static MASTER_ARCHIVE *gstInformationDB = NULL;

/*============================================================================*/
/*                           FUNCTION DEFINITIONS                             */
/*============================================================================*/

void cleanup(void)
{
    /* FREE MEMORY: Cleanup function for global resources */
    if (gstInformationDB != NULL) {
        /* Free any pending requests */
        if (gstInformationDB->pvRequests != NULL) {
            free(gstInformationDB->pvRequests);
            gstInformationDB->pvRequests = NULL;
        }
        /* Free the database itself */
        free(gstInformationDB);
        gstInformationDB = NULL;
    }
}

void setup(void)
{
    ERROR_CODE enErrorCode = ERR_OK;

    gstInformationDB = malloc(sizeof(MASTER_ARCHIVE));
    if (gstInformationDB != NULL) {
        memset(gstInformationDB, 0, sizeof(MASTER_ARCHIVE));
        gstInformationDB->enInstErrStatus = ERR_OK;
    }

    enErrorCode = initializeInterfaces();
    if (enErrorCode != ERR_OK) {
        print_err("%s:NetModInit<KO>ERR<%d>", __FUNCTION__, enErrorCode);
        return;
    }
}

void loop(void)
{
    startWorking();
    print_dbg("Work halted...");
}

#if OTA_HAL_SIMULATION
int main(void)
{
    setup();
    while (true) {
        loop();
    }
    return 0;
}
#endif

void startWorking(void)
{
    PVOID pvUReqBuffer = NULL;

    if (gstInformationDB == NULL) {
        print_err("MASTER Archive DB not initialized/NULL");
        return;
    }

    while (true) {
        
        pvUReqBuffer = receive_user_request(gstInformationDB);
        parse_request(gstInformationDB, pvUReqBuffer);
        initiate_order(gstInformationDB);
        actuate_motor(gstInformationDB);
        post_ack(gstInformationDB);
        
        /* FREE MEMORY LEAK: User request buffer allocated by HAL receive functions */
        if (pvUReqBuffer != NULL) {
            free(pvUReqBuffer);
            pvUReqBuffer = NULL;
        }
    }
}