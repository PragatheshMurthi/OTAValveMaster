#include "lora.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef OTA_HAL_SIMULATION

ERROR_CODE initialize_LoRa(void)
{
    print_info("%s:Simulation HAL enabled", __FUNCTION__);
    return ERR_OK;
}

ERROR_CODE receive_LoRa(void **ppvBuffer)
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
        print_info("%s:Simulated LoRa timeout/dropped packet", __FUNCTION__);
        return ERR_TIMEOUT;
    }

    /* Build ACK_BUFFER with variable statuses */
    ACK_BUFFER ack = {0};
    ack.u32OrderLength = sizeof(ACK_BUFFER);
    ack.u32SequenceNumber = (UINT32)(rand() & 0xFFFFu);
    ack.u32ValveID = 1u + (rand() % MAX_VALVE_ID);

    const char *ack_statuses[] = {"ORDER_PROCESSED", "ORDER_FAILED", "ORDER_PARTIAL", "ORDER_RETRY"};
    size_t status_count = sizeof(ack_statuses) / sizeof(ack_statuses[0]);
    const char *chosen = ack_statuses[rand() % status_count];
    strncpy(ack.acOrderStatus, chosen, MAX_ORDER_RESULT_LENGTH - 1);
    ack.acOrderStatus[MAX_ORDER_RESULT_LENGTH - 1] = '\0';

    ack.u32OrderCRC = calculate_crc(&ack, (UINT16)sizeof(ACK_BUFFER));

    *ppvBuffer = malloc(sizeof(ACK_BUFFER));
    if (*ppvBuffer == NULL) {
        return ERR_MEMORY_ALLOCATION_FAIL;
    }

    memcpy(*ppvBuffer, &ack, sizeof(ACK_BUFFER));

    print_info("%s:Simulated LoRa ACK produced Seq[%u] Valve[%u] Status[%s] Delay[%dms]", __FUNCTION__, ack.u32SequenceNumber, ack.u32ValveID, ack.acOrderStatus, delay_ms);
    print_dbg("%s:Receive<OK>Buf[%p]Len[%u]", __FUNCTION__, *ppvBuffer, (UINT32)sizeof(ACK_BUFFER));
    return ERR_OK;
}

#else

ERROR_CODE initialize_LoRa(void)
{
    print_info("%s:LoRa initialized", __FUNCTION__);
    return ERR_OK;
}

ERROR_CODE receive_LoRa(void **ppvBuffer)
{
    if (ppvBuffer == NULL) {
        return ERR_INVALID_PARAM;
    }

    *ppvBuffer = NULL;
    print_info("%s:Actual LoRa receive not implemented in this environment", __FUNCTION__);
    return ERR_OK;
}

#endif

ERROR_CODE send_LoRa(void *pvBuffer)
{
    (void)pvBuffer;
    print_dbg("%s:Simulated LoRa send (noop)", __FUNCTION__);
    return ERR_OK;
}