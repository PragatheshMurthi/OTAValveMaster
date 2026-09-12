#include "gsm.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef OTA_HAL_SIMULATION

ERROR_CODE initialize_GSM(void)
{
    print_info("%s:Simulation HAL enabled", __FUNCTION__);
    return ERR_OK;
}

ERROR_CODE receive_from_gsm(void **ppvBuffer)
{
    if (ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    *ppvBuffer = NULL;

    /* Random small delay to simulate network latency */
    int delay_ms = rand() % 500; /* up to 499ms */
    if (delay_ms > 0) {
        usleep(delay_ms * 1000);
    }

    /* Random chance of timeout/drop */
    int timeout_chance = rand() % 6; /* ~16% timeout */
    if (timeout_chance == 0) {
        print_info("%s:Simulated GSM timeout/dropped packet", __FUNCTION__);
        return ERR_TIMEOUT;
    }

    /* Build UREQ_METADATA + 1..3 UREQ_BUFFER entries */
    UINT32 num_requests = 1 + (rand() % 3);
    size_t meta_size = sizeof(UREQ_METADATA);
    size_t reqs_size = sizeof(UREQ_BUFFER) * num_requests;
    size_t total = meta_size + reqs_size;

    UINT8 *pbuf = malloc(total);
    if (pbuf == NULL) {
        return ERR_MEMORY_ALLOCATION_FAIL;
    }

    UREQ_METADATA *pmeta = (UREQ_METADATA *)pbuf;
    pmeta->u32NumberOfRequests = num_requests;
    pmeta->u32RequestSequenceNumber = (UINT32)(rand() & 0xFFFFu);
    pmeta->u32RequestLength = (UINT32)total;
    pmeta->u32RequestCRC = 0; /* will fill after payload */

    UREQ_BUFFER *preq = (UREQ_BUFFER *)(pbuf + meta_size);
    for (UINT32 i = 0; i < num_requests; ++i) {
        preq[i].u32ValveID = 1u + (rand() % MAX_VALVE_ID);
        preq[i].u8RequestType = (UINT32)(rand() % REQUEST_VALVE_STATUS + 1) - 1; /* ensure in-range */
        if (preq[i].u8RequestType == REQUEST_VALVE_OPEN_TIMED_CLOSE || preq[i].u8RequestType == REQUEST_VALVE_CLOSE_TIMED_OPEN) {
            preq[i].u32TimerDuration = MIN_TIMER_DURATION + (rand() % (MAX_TIMER_DURATION - MIN_TIMER_DURATION + 1));
        } else {
            preq[i].u32TimerDuration = 0u;
        }
    }

    /* Calculate CRC over full request buffer */
    pmeta->u32RequestCRC = calculate_crc(pbuf, (UINT16)total);

    *ppvBuffer = pbuf;

    print_info("%s:Simulated GSM user request Seq[%u] NR[%u] Delay[%dms]", __FUNCTION__, pmeta->u32RequestSequenceNumber, pmeta->u32NumberOfRequests, delay_ms);
    print_dbg("%s:Receive<OK>Buf[%p]Len[%u]", __FUNCTION__, *ppvBuffer, (UINT32)total);
    return ERR_OK;
}

#else

ERROR_CODE initialize_GSM(void)
{
    print_info("%s:GSM initialized", __FUNCTION__);
    return ERR_OK;
}

ERROR_CODE receive_from_gsm(void **ppvBuffer)
{
    if (ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    *ppvBuffer = NULL;
    print_info("%s:Actual GSM receive not implemented in this environment", __FUNCTION__);
    return ERR_OK;
}

#endif

ERROR_CODE send_gsm(void *pvBuffer)
{
    (void)pvBuffer;
    print_dbg("%s:Simulated GSM send (noop)", __FUNCTION__);
    return ERR_OK;
}