#include "comm.h"
#include "netintr.h"

static UINT32 calculate_crc_internal(const void *pvOrderBuffer, UINT16 u16OrderLength)
{
    const UINT8 *pucData = (const UINT8 *)pvOrderBuffer;
    UINT32 u32Crc = 0xFFFFFFFFu;

    if (pvOrderBuffer == NULL || u16OrderLength == 0U) {
        return 0U;
    }

    for (UINT16 u16Index = 0U; u16Index < u16OrderLength; ++u16Index) {
        u32Crc ^= (UINT32)pucData[u16Index];
        for (UINT8 u8Bit = 0U; u8Bit < 8U; ++u8Bit) {
            if ((u32Crc & 1U) != 0U) {
                u32Crc = (u32Crc >> 1U) ^ 0xEDB88320u;
            } else {
                u32Crc >>= 1U;
            }
        }
    }

    return (~u32Crc);
}

PVOID receive_user_request(MASTER_ARCHIVE *pstMasterArchive)
{
    ERROR_CODE enErrorCode = ERR_OK;
    PVOID pvUReqBuffer = NULL;

    DBG_ENTRY

    if (pstMasterArchive == NULL) {
        print_err("%s:MasterArchive<KO>", __FUNCTION__);
        DBG_EXIT
        return NULL;
    }

    if (get_error(pstMasterArchive) != ERR_OK) {
        print_err("%s:ErrorInPreviousState<KO><%d>", __FUNCTION__, get_error(pstMasterArchive));
        DBG_EXIT
        return NULL;
    }

    enErrorCode = hal_receive_from_user(&pvUReqBuffer);
    if (enErrorCode == ERR_OK && pvUReqBuffer != NULL) {
        print_dbg("%s:RequestRecieved<OK><OB[%p]><%d>", __FUNCTION__, pvUReqBuffer, enErrorCode);
        enErrorCode = evaluate_integrity(pvUReqBuffer);
        if (enErrorCode != ERR_OK) {
            print_err("%s:OrderIntegrityCheck<KO>ERR<%d>", __FUNCTION__, enErrorCode);
            set_error(pstMasterArchive, enErrorCode);
        }
    } else {
        print_err("%s:OrderRecieve<KO>ERR<%d>", __FUNCTION__, enErrorCode);
        set_error(pstMasterArchive, (enErrorCode == ERR_OK) ? ERR_TIMEOUT : enErrorCode);
    }

    print_info("%s:OrderRecieved<OK><OB[%p]><%d>", __FUNCTION__, pvUReqBuffer, enErrorCode);

    DBG_EXIT
    return pvUReqBuffer;
}

ERROR_CODE evaluate_integrity(PVOID pvUReqBuffer)
{
    UINT32 u32CalculatedCRC = 0U;
    UINT32 u32ReceivedCRC = 0U;
    UINT16 u16ReqLength = 0U;

    DBG_ENTRY

    if (pvUReqBuffer == NULL) {
        print_err("%s:ReqBuffer<KO><NULL>", __FUNCTION__);
        DBG_EXIT
        return ERR_INVALID_PARAM;
    }

    u16ReqLength = ((UREQ_METADATA *)pvUReqBuffer)->u32RequestLength;
    u32ReceivedCRC = ((UREQ_METADATA *)pvUReqBuffer)->u32RequestCRC;
    print_dbg("%s:ReqIntegrityCheck<OK><OB[%p]OL[%d]RC[%d]>", __FUNCTION__, pvUReqBuffer, u16ReqLength, u32ReceivedCRC);

    ((UREQ_METADATA *)pvUReqBuffer)->u32RequestCRC = 0U; /* Clear CRC field before calculation */

    u32CalculatedCRC = calculate_crc(pvUReqBuffer, u16ReqLength);
    if (u32ReceivedCRC != u32CalculatedCRC) {
        print_err("%s:ReqIntegrityCheck<KO><OB[%p]OL[%d]RC[%d]CC[%d]>", __FUNCTION__, pvUReqBuffer, u16ReqLength, u32ReceivedCRC, u32CalculatedCRC);
        DBG_EXIT
        return ERR_INTEGRITY_CHECK_FAILED;
    }

    DBG_EXIT
    return ERR_OK;
}

UINT32 calculate_crc(const void *pvOrderBuffer, UINT16 u16OrderLength)
{
    return calculate_crc_internal(pvOrderBuffer, u16OrderLength);
}

void initiate_order(MASTER_ARCHIVE *pstMasterArchive)
{
    ERROR_CODE enErrorCode = ERR_OK;
    UINT32 u32RequestsLooper = 0U;
    SLAVE_ORDER_BUFFER stOrderBuffer = {0};
    ACK_BUFFER stAckBuffer = {0};
    UREQ_BUFF_INTERNAL *pstRecAckData = NULL;

    DBG_ENTRY

    if (pstMasterArchive == NULL) {
        print_err("%s:MasterArchive<KO>", __FUNCTION__);
        DBG_EXIT
        return;
    }

    if (get_error(pstMasterArchive) != ERR_OK) {
        print_err("%s:ErrorInPreviousState<KO><%d>", __FUNCTION__, get_error(pstMasterArchive));
        DBG_EXIT
        return;
    }

    while (pstMasterArchive->u16NumberOfRequests > u32RequestsLooper) {
        UREQ_BUFF_INTERNAL *pstCurrRequest = ((UREQ_BUFF_INTERNAL *)pstMasterArchive->pvRequests) + u32RequestsLooper;

        stOrderBuffer.u32ValveNumber = pstCurrRequest->u32ValveID;
        stOrderBuffer.u32OrderSequenceNumber = pstMasterArchive->u32CurrSequence;
        stOrderBuffer.u8ActionType = (UINT8)pstCurrRequest->u8RequestType;
        stOrderBuffer.u32TimerCntS = pstCurrRequest->u32TimerDuration;
        stOrderBuffer.u32OrderLength = sizeof(stOrderBuffer);
        stOrderBuffer.u32OrderCRC = calculate_crc(&stOrderBuffer, sizeof(stOrderBuffer));

        hal_send_to_slave(&stOrderBuffer, sizeof(stOrderBuffer));
        pstCurrRequest->u8OrderStatus = ORDER_STATUS_ACK_PENDING;

        enErrorCode = hal_receive_from_slave(&stAckBuffer, MAX_ACK_WAIT_TIME);
        if (enErrorCode == ERR_OK) {
            print_dbg("%s:AckRecieved<OK><AB[%p]><%d>", __FUNCTION__, &stAckBuffer, enErrorCode);

            if (stAckBuffer.u32SequenceNumber != pstMasterArchive->u32CurrSequence) {
                print_dbg("%s:AckSequenceMatch<OK><AB[%p]SN[%d]CN[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.u32SequenceNumber, pstMasterArchive->u32CurrSequence);
                pstRecAckData = find_instance_by_valve_id(pstMasterArchive, stAckBuffer.u32ValveID);
                if (pstRecAckData == NULL) {
                    print_err("%s:NoValidInst<KO><AB[%p]AS[%s]VID[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus, stAckBuffer.u32ValveID);
                } else {
                    if (strcmp(stAckBuffer.acOrderStatus, "ORDER_PROCESSED") == 0) {
                        print_dbg("%s:AckStatus<OK><AB[%p]AS[%s]VID[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus, stAckBuffer.u32ValveID);
                        strncpy(pstRecAckData->acOrderResult, "Success", MAX_ORDER_RESULT_LENGTH - 1);
                        pstRecAckData->acOrderResult[MAX_ORDER_RESULT_LENGTH - 1] = '\0';
                        pstMasterArchive->u16PositiveAckCount++;
                    } else {
                        print_dbg("%s:AckStatus<OK><AB[%p]AS[%s]VID[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus, stAckBuffer.u32ValveID);
                        strncpy(pstRecAckData->acOrderResult, stAckBuffer.acOrderStatus, MAX_ORDER_RESULT_LENGTH - 1);
                        pstRecAckData->acOrderResult[MAX_ORDER_RESULT_LENGTH - 1] = '\0';
                        print_err("%s:AckStatus<KO><AB[%p]AS[%s]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus);
                    }
                }
            } else {
                print_err("%s:AckSequenceMismatch<KO><AB[%p]SN[%d]CN[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.u32SequenceNumber, pstMasterArchive->u32CurrSequence);
            }
        } else {
            print_err("%s:AckReceive<KO>ERR<%d>", __FUNCTION__, enErrorCode);
        }

        u32RequestsLooper++;
    }

    print_dbg("%s:OrderInitiated<OK><GA[%p]SN[%d]CN[%d]NR[%d]>", __FUNCTION__, pstMasterArchive, pstMasterArchive->u32CurrSequence, pstMasterArchive->u32PrevSequence, pstMasterArchive->u16NumberOfRequests);
    DBG_EXIT
}

UREQ_BUFF_INTERNAL *find_instance_by_valve_id(MASTER_ARCHIVE *pstMasterArchive, UINT32 u32ValveID)
{
    UINT32 u32RequestsLooper = 0U;
    UREQ_BUFF_INTERNAL *pstCurrUReqData = NULL;

    if (pstMasterArchive == NULL) {
        print_err("%s:MasterArchive<KO><NULL>", __FUNCTION__);
        set_error(pstMasterArchive, ERR_INVALID_PARAM);
        return NULL;
    }

    while (u32RequestsLooper < pstMasterArchive->u16NumberOfRequests) {
        pstCurrUReqData = ((UREQ_BUFF_INTERNAL *)pstMasterArchive->pvRequests) + u32RequestsLooper;
        if (pstCurrUReqData->u32ValveID == u32ValveID) {
            print_dbg("%s:MatchingRequestFound<OK><GA[%p]VID[%d]NR[%d]>", __FUNCTION__, pstMasterArchive, u32ValveID, pstMasterArchive->u16NumberOfRequests);
            return pstCurrUReqData;
        }
        u32RequestsLooper++;
    }

    print_err("%s:NoMatchingRequest<KO><GA[%p]VID[%d]NR[%d]>", __FUNCTION__, pstMasterArchive, u32ValveID, pstMasterArchive->u16NumberOfRequests);
    return NULL;
}

void post_ack(MASTER_ARCHIVE *pstMasterArchive)
{
    ERROR_CODE enErrorCode = ERR_OK;
    USR_ACK_BUFFER stAckBuffer = {0};

    DBG_ENTRY

    if (pstMasterArchive == NULL) {
        print_err("%s:pstMasterArchive<KO>", __FUNCTION__);
        DBG_EXIT
        return;
    }

    stAckBuffer.u32SequenceNumber = pstMasterArchive->u32CurrSequence;
    if (pstMasterArchive->u16PositiveAckCount != pstMasterArchive->u16NumberOfRequests) {
        stAckBuffer.u32RequestState = ORDER_STATUS_FAILED;
        strncpy(stAckBuffer.acRequestStatus, "ORDER_FAILED", MAX_USR_ACK_RESULT_LENGTH - 1U);
        stAckBuffer.acRequestStatus[MAX_USR_ACK_RESULT_LENGTH - 1U] = '\0';
    } else {
        stAckBuffer.u32RequestState = ORDER_STATUS_ACK_RECEIVED;
        strncpy(stAckBuffer.acRequestStatus, "ORDER_SUCCESS", MAX_USR_ACK_RESULT_LENGTH - 1U);
        stAckBuffer.acRequestStatus[MAX_USR_ACK_RESULT_LENGTH - 1U] = '\0';
    }

    if (fill_ack_buffer(stAckBuffer.acRequestStatus, MAX_USR_ACK_RESULT_LENGTH, pstMasterArchive) != ERR_OK) {
        strncpy(stAckBuffer.acRequestStatus, "STATUS_POPULATION_FAILED", MAX_USR_ACK_RESULT_LENGTH - 1U);
        stAckBuffer.acRequestStatus[MAX_USR_ACK_RESULT_LENGTH - 1U] = '\0';
        print_err("%s:FailedToFillAckBuffer<KO>", __FUNCTION__);
    }

    stAckBuffer.u32OrderCRC = calculate_crc(&stAckBuffer, sizeof(stAckBuffer));
    stAckBuffer.u32OrderLength = sizeof(stAckBuffer);
    enErrorCode = hal_send_to_user(&stAckBuffer, sizeof(stAckBuffer));

    if (enErrorCode == ERR_OK) {
        print_dbg("%s:AckSent<OK><AB[%p]><%d>", __FUNCTION__, &stAckBuffer, enErrorCode);
    } else {
        print_err("%s:AckSend<KO>ERR<%d>", __FUNCTION__, enErrorCode);
    }

    DBG_EXIT
}

ERROR_CODE fill_ack_buffer(CHAR *pchAckStatus, UINT16 u16AckLength, MASTER_ARCHIVE *pstMasterArchive)
{
    if (pchAckStatus == NULL || pstMasterArchive == NULL) {
        print_err("%s:AckBufferOrGlobalArchive<KO><AB[%p]GA[%p]>", __FUNCTION__, pchAckStatus, pstMasterArchive);
        return ERR_INVALID_PARAM;
    }

    if (u16AckLength > 1U) {
        pchAckStatus[u16AckLength - 1U] = '\0';
    }

    print_dbg("%s:AckBufferFilled<OK><AB[%p]SN[%d]>", __FUNCTION__, pchAckStatus, pstMasterArchive->u32CurrSequence);
    return ERR_OK;
}

void parse_request(MASTER_ARCHIVE *pstMasterArchive, PVOID pvUReqBuffer)
{
    UREQ_BUFFER *apstCurrUReqData = NULL;
    UREQ_BUFFER *apstUReqBase = NULL;
    UREQ_BUFF_INTERNAL *apstUReqData = NULL;

    DBG_ENTRY

    if (pstMasterArchive == NULL || pvUReqBuffer == NULL) {
        set_error(pstMasterArchive, ERR_INVALID_PARAM);
        print_err("%s:<KO-ADD><GA[%p]OB[%p]>", __FUNCTION__, pstMasterArchive, pvUReqBuffer);
        DBG_EXIT
        return;
    }

    if (get_error(pstMasterArchive) != ERR_OK) {
        print_err("%s:ErrorInPreviousState<KO><%d>", __FUNCTION__, get_error(pstMasterArchive));
        DBG_EXIT
        return;
    }

    if (((UREQ_METADATA *)pvUReqBuffer)->u32RequestSequenceNumber <= pstMasterArchive->u32CurrSequence) {
        print_dbg("%s:RequestSequenceNumber<KO><OB[%p]SN[%d]CN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA *)pvUReqBuffer)->u32RequestSequenceNumber, pstMasterArchive->u32CurrSequence);
        set_error(pstMasterArchive, ERR_STALE_ORDER);
        DBG_EXIT
        return;
    }

    if (((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests < 1U || ((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests >= MAX_USER_REQUESTS) {
        print_dbg("%s:Requests over/under flow<KO><OB[%p]SN[%d]CN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests, pstMasterArchive->u32CurrSequence);
        set_error(pstMasterArchive, ERR_ORDER_OOB);
        DBG_EXIT
        return;
    }

    apstUReqData = malloc(sizeof(UREQ_BUFF_INTERNAL) * ((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests);
    if (apstUReqData == NULL) {
        set_error(pstMasterArchive, ERR_MEMORY_ALLOCATION_FAIL);
        print_dbg("%s:MemoryAllocation<KO><NO[%d]CS[%d]>", __FUNCTION__, ((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests, pstMasterArchive->u32CurrSequence);
        DBG_EXIT
        return;
    }

    memset(apstUReqData, 0, sizeof(UREQ_BUFF_INTERNAL) * ((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests);
    apstUReqBase = (UREQ_BUFFER *)((UINT8 *)pvUReqBuffer + sizeof(UREQ_METADATA));

    for (UINT32 u32RequestsLooper = 0U; u32RequestsLooper < ((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests; ++u32RequestsLooper) {
        UREQ_BUFF_INTERNAL stPopulationTemp = {0};
        apstCurrUReqData = apstUReqBase + u32RequestsLooper;

        if (populate_request(apstCurrUReqData, &stPopulationTemp) == ERR_OK) {
            memcpy(apstUReqData + u32RequestsLooper, &stPopulationTemp, sizeof(UREQ_BUFF_INTERNAL));
            print_dbg("%s:RequestParsed<OK><OB[%p]SN[%d]CN[%d]RN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA *)pvUReqBuffer)->u32RequestSequenceNumber, pstMasterArchive->u32CurrSequence, u32RequestsLooper);
        } else {
            print_err("%s:RequestValidation<KO><OB[%p]SN[%d]CN[%d]RN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA *)pvUReqBuffer)->u32RequestSequenceNumber, pstMasterArchive->u32CurrSequence, u32RequestsLooper);
            set_error(pstMasterArchive, ERR_INVALID_REQUEST);
        }
    }

    pstMasterArchive->pvRequests = apstUReqData;
    pstMasterArchive->u16NumberOfRequests = (UINT16)((UREQ_METADATA *)pvUReqBuffer)->u32NumberOfRequests;
    pstMasterArchive->u32CurrSequence = ((UREQ_METADATA *)pvUReqBuffer)->u32RequestSequenceNumber;
    print_dbg("%s:MasterArchiveUpdated<OK><GA[%p]SN[%d]CN[%d]NR[%d]>", __FUNCTION__, pstMasterArchive, ((UREQ_METADATA *)pvUReqBuffer)->u32RequestSequenceNumber, pstMasterArchive->u32CurrSequence, pstMasterArchive->u16NumberOfRequests);
    DBG_EXIT
}

ERROR_CODE populate_request(UREQ_BUFFER *pstSRCUReqData, UREQ_BUFF_INTERNAL *pstDestUReqData)
{
    DBG_ENTRY

    if (pstSRCUReqData == NULL || pstDestUReqData == NULL) {
        print_err("%s:RequestBuffer<KO><SRC[%p]DEST[%p]>", __FUNCTION__, pstSRCUReqData, pstDestUReqData);
        DBG_EXIT
        return ERR_INVALID_PARAM;
    }

    if (pstSRCUReqData->u32ValveID < 1U || pstSRCUReqData->u32ValveID > MAX_VALVE_ID) {
        print_err("%s:ValveID<KO><VID[%d]MAX[%d]>", __FUNCTION__, pstSRCUReqData->u32ValveID, MAX_VALVE_ID);
        DBG_EXIT
        return ERR_INVALID_REQUEST;
    }

    pstDestUReqData->u32ValveID = pstSRCUReqData->u32ValveID;

    if (pstSRCUReqData->u8RequestType == REQUEST_VALVE_OPEN) {
        pstDestUReqData->u8RequestType = REQUEST_VALVE_OPEN;
        pstDestUReqData->u32TimerDuration = 0U;
    } else if (pstSRCUReqData->u8RequestType == REQUEST_VALVE_CLOSE) {
        pstDestUReqData->u8RequestType = REQUEST_VALVE_CLOSE;
        pstDestUReqData->u32TimerDuration = 0U;
    } else if (pstSRCUReqData->u8RequestType == REQUEST_VALVE_OPEN_TIMED_CLOSE || pstSRCUReqData->u8RequestType == REQUEST_VALVE_CLOSE_TIMED_OPEN) {
        if (pstSRCUReqData->u32TimerDuration < MIN_TIMER_DURATION || pstSRCUReqData->u32TimerDuration > MAX_TIMER_DURATION) {
            print_err("%s:TimerDuration<KO><TD[%d]MIN[%d]MAX[%d]>", __FUNCTION__, pstSRCUReqData->u32TimerDuration, MIN_TIMER_DURATION, MAX_TIMER_DURATION);
            DBG_EXIT
            return ERR_INVALID_REQUEST;
        }

        pstDestUReqData->u8RequestType = pstSRCUReqData->u8RequestType;
        pstDestUReqData->u32TimerDuration = pstSRCUReqData->u32TimerDuration;
    } else if (pstSRCUReqData->u8RequestType == REQUEST_VALVE_STATUS) {
        pstDestUReqData->u8RequestType = REQUEST_VALVE_STATUS;
        pstDestUReqData->u32TimerDuration = 0U;
    } else {
        print_err("%s:RequestValidation<KO><VID[%d]RT[%d]>", __FUNCTION__, pstSRCUReqData->u32ValveID, pstSRCUReqData->u8RequestType);
        DBG_EXIT
        return ERR_INVALID_REQUEST;
    }

    print_dbg("%s:RequestValidated<OK><VID[%d]RT[%d]TD[%d]>", __FUNCTION__, pstDestUReqData->u32ValveID, pstDestUReqData->u8RequestType, pstDestUReqData->u32TimerDuration);
    DBG_EXIT
    return ERR_OK;
}