#include "motor.h"
#include "comm.h"

void actuate_motor(MASTER_ARCHIVE *pstMasterArchive)
{
    DBG_ENTRY

    if (pstMasterArchive == NULL) {
        print_err("%s:MASTER Archive<KO>", __FUNCTION__);
        DBG_EXIT
        return;
    }

    if (pstMasterArchive->pvRequests != NULL && pstMasterArchive->u16NumberOfRequests > 0U) {
        UREQ_BUFF_INTERNAL *pstRequestList = (UREQ_BUFF_INTERNAL *)pstMasterArchive->pvRequests;
        for (UINT16 u16Index = 0U; u16Index < pstMasterArchive->u16NumberOfRequests; ++u16Index) {
            switch (pstRequestList[u16Index].u8RequestType) {
                case REQUEST_VALVE_OPEN:
                    print_info("%s:Valve[%u] OPEN requested", __FUNCTION__, pstRequestList[u16Index].u32ValveID);
                    break;
                case REQUEST_VALVE_CLOSE:
                    print_info("%s:Valve[%u] CLOSE requested", __FUNCTION__, pstRequestList[u16Index].u32ValveID);
                    break;
                case REQUEST_VALVE_OPEN_TIMED_CLOSE:
                    print_info("%s:Valve[%u] OPEN_TIMED_CLOSE[%u] requested", __FUNCTION__, pstRequestList[u16Index].u32ValveID, pstRequestList[u16Index].u32TimerDuration);
                    break;
                case REQUEST_VALVE_CLOSE_TIMED_OPEN:
                    print_info("%s:Valve[%u] CLOSE_TIMED_OPEN[%u] requested", __FUNCTION__, pstRequestList[u16Index].u32ValveID, pstRequestList[u16Index].u32TimerDuration);
                    break;
                case REQUEST_VALVE_STATUS:
                    print_info("%s:Valve[%u] STATUS requested", __FUNCTION__, pstRequestList[u16Index].u32ValveID);
                    break;
                default:
                    print_err("%s:Unknown valve action[%u] for valve[%u]", __FUNCTION__, pstRequestList[u16Index].u8RequestType, pstRequestList[u16Index].u32ValveID);
                    break;
            }
        }
    }

    DBG_EXIT
}