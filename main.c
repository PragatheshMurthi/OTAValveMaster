#include <stdbool.h>

#include "comm.h"
#include "motor.h"
#include "netintr.h"

void startWorking(void);

static MASTER_ARCHIVE *gstInformationDB = NULL;

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
    if (gstInformationDB == NULL) {
        print_err("MASTER Archive DB not initialized/NULL");
        return;
    }

    while (true) {
        PVOID pvUReqBuffer = NULL;

        pvUReqBuffer = receive_user_request(gstInformationDB);
        parse_request(gstInformationDB, pvUReqBuffer);
        initiate_order(gstInformationDB);
        actuate_motor(gstInformationDB);
        post_ack(gstInformationDB);
    }
}