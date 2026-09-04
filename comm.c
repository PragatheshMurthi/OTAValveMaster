/* ============= INCLUDES ============= */
#include "comm.h"

/* ============= Function Definitions ============= */


/* ============== RECEIVER IMPLEMENTATION ============== */

/*
 * Expects an order from the network and processes it.
 *
 * Parameters:
 *   pstGlobalArchive - Pointer to the global archive structure.
 * 
 */
PVOID receive_user_request ( MASTER_ARCHIVE* pstMasterArchive )
{
    ERROR_CODE enErroCode = ERR_OK;
    PVOID pvUReqBuffer = NULL;

    DBG_ENTRY

    if ( NULL == pstMasterArchive )
    {
        set_error(ERR_INVALID_PARAM);
        print_err("%s:MasterArchive<KO>", __FUNCTION__);
        DBG_EXIT
        return NULL;
    }

    if ( get_error( pstMasterArchive ) != ERR_OK )
    {
        print_err("%s:ErrorInPreviousState<KO><%d>", __FUNCTION__, get_error( pstMasterArchive ));
        DBG_EXIT
        return NULL;
    }
    
    enErroCode = hal_receive( pvUReqBuffer );

    if ( enErrorCode == ERR_OK && NULL != pvUReqBuffer )
    {
        print_dbg("%s:RequestRecieved<OK><OB[%p]><%d>", __FUNCTION__, pvUReqBuffer, enErrorCode);
        enErrorCode = evaluate_integrity( pvOrderBuffer );

        if ( enErrorCode != ERR_OK )
        {
            // Handle integrity check failure
            print_err("%s:OrderIntegrityCheck<KO>ERR<%d>", __FUNCTION__, enErrorCode);
            set_error( enErrorCode );
        }
    } else {
        print_err("%s:OrderRecieve<KO>ERR<%d>", __FUNCTION__, enErrorCode);
        set_error( enErrorCode );
    }

    print_info("%s:OrderRecieved<OK><OB[%p]><%d>", __FUNCTION__, pvOrderBuffer, enErrorCode);

    DBG_EXIT
    return pvOrderBuffer;
}

/*
 * Evaluates the integrity of the received order using CRC.
 *
 * Parameters:
 *   pvOrderBuffer - Pointer to the order buffer.
 * Returns:
 *   ERROR_CODE - Status of the integrity check.
 */
ERROR_CODE evaluate_integrity( PVOID pvUReqBuffer )
{
    UINT32 u32CalculatedCRC = 0;
    UINT32 u32ReceivedCRC   = 0;
    UINT16 u16ReqLength   = 0;

    DBG_ENTRY

    if ( NULL == pvUReqBuffer )
    {
        print_err("%s:ReqBuffer<KO><NULL>", __FUNCTION__);
        set_error(ERR_INVALID_PARAM);
        DBG_EXIT
        return ERR_INVALID_PARAM;
    }

    u16ReqLength = ((UREQ_METADATA*)pvUReqBuffer)->u32RequestLength;
    u32ReceivedCRC = ((UREQ_METADATA*)pvUReqBuffer)->u32RequestCRC;

    print_dbg("%s:ReqIntegrityCheck<OK><OB[%p]OL[%d]RC[%d]>", __FUNCTION__, pvUReqBuffer, u16ReqLength, u32ReceivedCRC);

    u32CalculatedCRC = calculate_crc( pvReqBuffer, u16ReqLength );

    if ( u32ReceivedCRC != u32CalculatedCRC )
    {
        print_err("%s:ReqIntegrityCheck<KO><OB[%p]OL[%d]RC[%d]CC[%d]>", __FUNCTION__, pvReqBuffer, u16ReqLength, u32ReceivedCRC, u32CalculatedCRC);
        set_error(ERR_INTEGRITY_CHECK_FAILED);
        DBG_EXIT
        return ERR_INTEGRITY_CHECK_FAILED;
    }

    DBG_EXIT
    return ERR_OK;
}

/*
 * Calculates the CRC for the given order buffer.
 *
 * Parameters:
 *   pvOrderBuffer - Pointer to the order buffer.
 *   u16OrderLength - Length of the order.
 * Returns:
 *   UINT32 - Calculated CRC value.
 */
UINT32 calculate_crc( PVOID pvOrderBuffer, UINT16 u16OrderLength )
{
    // Note: ~0UL initializes the context to 0xFFFFFFFF
    // esp_rom_crc32_le calculates little-endian CRC-32 (standard Ethernet IEEE 802.3)
    return esp_rom_crc32_le(~0UL, pvOrderBuffer, u16OrderLength) ^ 0xFFFFFFFF;
}

/* ============== SEND IMPLEMENTATION ============== */

VOID initiate_order( MASTER_ARCHIVE* pstMasterArchive )
{
    ERROR_CODE enErrorCode = ERR_OK;

    DBG_ENTRY

    if ( NULL == pstMasterArchive )
    {
        set_error(ERR_INVALID_PARAM);
        print_err("%s:MasterArchive<KO>", __FUNCTION__);
        DBG_EXIT
        return;
    }

    if ( get_error( pstMasterArchive ) != ERR_OK )
    {
        print_err("%s:ErrorInPreviousState<KO><%d>", __FUNCTION__, get_error( pstMasterArchive ));
        DBG_EXIT
        return;
    }

    // Initiate the order processing based on the parsed requests.
    // This is a placeholder for actual order initiation logic.
    // For example, you might want to send commands to valves or other actuators here.
    UINT32 u32RequestsLooper = 0;
    UINT32 u32CurrTime      = 0,
           u32ElapsedTime    = 0;
    SLAVE_ORDER_BUFFER stOrderBuffer = { 0 };

    while ( pstMasterArchive->u16NumberOfRequests > u32RequestsLooper )
    {
        // Process each request in the order.
        // This is a placeholder for actual request processing logic.
        // For example, you might want to send commands to valves or other actuators here.
        stOrderBuffer.u32ValveNumber = ((UREQ_BUFF_INTERNAL*)(pstMasterArchive->pvRequests) + u32RequestsLooper)->u32ValveID;
        stOrderBuffer.u32OrderSequenceNumber = pstMasterArchive->u32CurrSequence;
        stOrderBuffer.u8ActionType = ((UREQ_BUFF_INTERNAL*)(pstMasterArchive->pvRequests) + u32RequestsLooper)->u8RequestType;
        stOrderBuffer.u32TimerCntS = ((UREQ_BUFF_INTERNAL*)(pstMasterArchive->pvRequests) + u32RequestsLooper)->u32TimerDuration;

        hal_send( &stOrderBuffer, sizeof(SLAVE_ORDER_BUFFER) );
        
        ((UREQ_BUFF_INTERNAL*)(pstMasterArchive->pvRequests) + u32RequestsLooper)->u8OrderStatus = ORDER_STATUS_ACK_PENDING;
        
        // After processing, increment the number of requests.
        u32RequestsLooper++;
    }

    // Acknowledge the order initiation and wait for acknowledgments from the slave devices.

    get_current_time(&u32CurrTime);

    while ( true )
    {
        // Wait for acknowledgment from the slave device.
        // This is a placeholder for actual acknowledgment handling logic.
        // For example, you might want to wait for a response from the slave device here.
        ACK_BUFFER stAckBuffer = { 0 };
        enErrorCode = hal_receive( &stAckBuffer );
        UREQ_BUFF_INTERNAL *pstRecAckData = null_ptr;

        if ( enErrorCode == ERR_OK )
        {
            print_dbg("%s:AckRecieved<OK><AB[%p]><%d>", __FUNCTION__, &stAckBuffer, enErrorCode);

            // Validate the acknowledgment sequence number against the current sequence number in the master archive.
            if (stAckBuffer.u32OrderSequenceNumber != pstMasterArchive->u32CurrSequence)
            {
                print_dbg("%s:AckSequenceMatch<OK><AB[%p]SN[%d]CN[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.u32OrderSequenceNumber, pstMasterArchive->u32CurrSequence);

                // Find the corresponding request in the master archive based on the valve ID and update its status.
                pstRecAckData = find_instance_by_valve_id( pstMasterArchive, stAckBuffer.u32ValveID );
                if ( null_ptr == pstRecAckData )
                {
                    print_err("%s:NoValidInst<KO><AB[%p]AS[%s]VID[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus, stAckBuffer.u32ValveID);
                    
                } else {
                    // Check the acknowledgment status and update the corresponding request in the master archive.
                    if ( strcmp(stAckBuffer.acOrderStatus, "ORDER_PROCESSED") == 0 )
                    {
                        print_dbg("%s:AckStatus<OK><AB[%p]AS[%s]VID[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus, stAckBuffer.u32ValveID);
                        strncpy (pstRecAckData->acOrderResult, "Success", MAX_ORDER_RESULT_LENGTH - 1);
                        pstRecAckData->acOrderResult[MAX_ORDER_RESULT_LENGTH - 1] = '\0'; // Ensure null-termination
                        pstMasterArchive->u16PositiveAckCount++;
                        
                    } else {

                        print_dbg("%s:AckStatus<OK><AB[%p]AS[%s]VID[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus, stAckBuffer.u32ValveID);
                        strncpy(pstRecAckData->acOrderResult, stAckBuffer.acOrderStatus, MAX_ORDER_RESULT_LENGTH - 1);
                        pstRecAckData->acOrderResult[MAX_ORDER_RESULT_LENGTH - 1] = '\0'; // Ensure null-termination
                    
                        print_err("%s:AckStatus<KO><AB[%p]AS[%s]>", __FUNCTION__, &stAckBuffer, stAckBuffer.acOrderStatus);
                    
                    }
                }
            } else {
                print_err("%s:AckSequenceMismatch<KO><AB[%p]SN[%d]CN[%d]>", __FUNCTION__, &stAckBuffer, stAckBuffer.u32OrderSequenceNumber, pstMasterArchive->u32CurrSequence);
            }
        } else {
            print_err("%s:AckReceive<KO>ERR<%d>", __FUNCTION__, enErrorCode);
        }

        // loop break condition: If all requests have been acknowledged, break the loop.
        if ( pstMasterArchive->u16PositiveAckCount >= pstMasterArchive->u16NumberOfRequests )
        {
            print_dbg("%s:AllAcksReceived<OK><GA[%p]SN[%d]CN[%d]NR[%d]>", 
                        __FUNCTION__, 
                        pstMasterArchive, 
                        pstMasterArchive->u32CurrSequence, 
                        pstMasterArchive->u32PrevSequence, 
                        pstMasterArchive->u16NumberOfRequests);
            break;
        }

        // loop break condition: If the acknowledgment wait time exceeds the maximum wait time, break the loop.
        if ( get_current_time(&u32ElapsedTime) - u32CurrTime > MAX_ACK_WAIT_TIME )
        {
            print_err("%s:AckWaitTimeout<KO><AB[%p]ET[%d]MWT[%d]>", __FUNCTION__, &stAckBuffer, u32ElapsedTime, MAX_ACK_WAIT_TIME);
            set_error(ERR_TIMEOUT);
            break;
        }
    }

    print_dbg("%s:OrderInitiated<OK><GA[%p]SN[%d]CN[%d]NR[%d]>", 
                __FUNCTION__, 
                pstMasterArchive, 
                pstMasterArchive->u32CurrSequence, 
                pstMasterArchive->u32PrevSequence, 
                pstMasterArchive->u16NumberOfRequests);

    DBG_EXIT
    return;
}

UREQ_BUFF_INTERNAL* find_instance_by_valve_id( MASTER_ARCHIVE* pstMasterArchive, UINT32 u32ValveID )
{
    UINT32 u32RequestsLooper = 0;
    UREQ_BUFF_INTERNAL *pstCurrUReqData = null_ptr;

    if ( NULL == pstMasterArchive )
    {
        print_err("%s:MasterArchive<KO><NULL>", __FUNCTION__);
        set_error(ERR_INVALID_PARAM);
        return null_ptr;
    }

    while ( u32RequestsLooper < pstMasterArchive->u16NumberOfRequests )
    {
        pstCurrUReqData = ((UREQ_BUFF_INTERNAL*)(pstMasterArchive->pvRequests) + u32RequestsLooper);
        if ( pstCurrUReqData->u32ValveID == u32ValveID )
        {
            print_dbg("%s:MatchingRequestFound<OK><GA[%p]VID[%d]NR[%d]>", __FUNCTION__, pstMasterArchive, u32ValveID, pstMasterArchive->u16NumberOfRequests);
            return pstCurrUReqData; // Return the pointer to the matching request.
        }
        u32RequestsLooper++;
    }

    print_err("%s:NoMatchingRequest<KO><GA[%p]VID[%d]NR[%d]>", __FUNCTION__, pstMasterArchive, u32ValveID, pstMasterArchive->u16NumberOfRequests);
    return null_ptr; // No matching request found.
}

VOID post_ack ( MASTER_ARCHIVE* pstMasterArchive )
{
    ERROR_CODE enErrorCode = ERR_OK;
    USR_ACK_BUFFER stAckBuffer = { 0 };

    DBG_ENTRY

    if ( NULL == pstMasterArchive )
    {
        set_error(pstMasterArchive, ERR_INVALID_PARAM);
        print_err("%s:pstMasterArchive<KO>", __FUNCTION__);
        DBG_EXIT
        return;
    }
    
    stAckBuffer.u32SequenceNumber = pstMasterArchive->u32CurrSequence;

    if ( pstMasterArchive->u16PositiveAckCount != pstMasterArchive->u16NumberOfRequests )
    {
        stAckBuffer.u32RequestState = ORDER_STATUS_FAILED;
        strncpy(&(stAckBuffer.acRequestStatus), "ORDER_FAILED", MAX_USR_ACK_RESULT_LENGTH - 1);
        stAckBuffer.acRequestStatus[MAX_USR_ACK_RESULT_LENGTH - 1] = '\0';
    } else {
        stAckBuffer.u32RequestState = ORDER_STATUS_ACK_RECEIVED;
        strncpy(&(stAckBuffer.acRequestStatus), "ORDER_SUCCESS", MAX_USR_ACK_RESULT_LENGTH - 1);
        stAckBuffer.acRequestStatus[MAX_USR_ACK_RESULT_LENGTH - 1] = '\0';
    }

    if ( fill_ack_buffer( &stAckBuffer.acRequestStatus, MAX_USR_ACK_RESULT_LENGTH, pstMasterArchive ) != ERR_OK )
    {
        strncpy(&(stAckBuffer.acRequestStatus), "STATUS_POPULATION_FAILED", MAX_USR_ACK_RESULT_LENGTH - 1);
        stAckBuffer.acOrderStatus[MAX_USR_ACK_RESULT_LENGTH - 1] = '\0';
        print_err("%s:FailedToFillAckBuffer<KO>", __FUNCTION__);
    }

    stAckBuffer.u32OrderCRC = calculate_crc( &stAckBuffer, sizeof(stAckBuffer) );
    stAckBuffer.u32OrderLength = sizeof(stAckBuffer);
    
    enErrorCode = hal_send( &stAckBuffer, sizeof(stAckBuffer) );

    if ( enErrorCode == ERR_OK && NULL != &stAckBuffer )
    {
        print_dbg("%s:AckSent<OK><AB[%p]><%d>", __FUNCTION__, &stAckBuffer, enErrorCode);
    } else {
        print_err("%s:AckSend<KO>ERR<%d>", __FUNCTION__, enErrorCode);
    }

    DBG_EXIT
    return enErrorCode;
}

ERROR_CODE fill_ack_buffer( PCHAR pchAckStatus, UINT16 u16AckLength, MASTER_ARCHIVE* pstMasterArchive )
{
    if ( NULL == pchAckStatus || NULL == pstMasterArchive )
    {
        print_err("%s:AckBufferOrGlobalArchive<KO><AB[%p]GA[%p]>", __FUNCTION__, pchAckStatus, pstMasterArchive);
        return ERR_INVALID_PARAM;
    }

    

    print_dbg("%s:AckBufferFilled<OK><AB[%p]VN[%d]SN[%d]AT[%d]TC[%d]>", __FUNCTION__, pstAckBuffer, pstAckBuffer->u32VaulveNumber, pstAckBuffer->u32OrderSequenceNumber, pstAckBuffer->u8ActionType, pstAckBuffer->u32TimerCntS);

    return ERR_OK;
}

/* ============== PARSER IMPLEMENTATION ============== */

/*
 * Parses the received order and updates the global archive with order details.
 *
 * Parameters:
 *  pstGlobalArchive - Pointer to the global archive structure.
 *  pvOrderBuffer - Pointer to the order buffer.
 */
VOID parse_request( MASTER_ARCHIVE* pstMasterArchive, PVOID pvUReqBuffer )
{
    
    DBG_ENTRY

    UREQ_BUFFER* apstCurrUReqData = null_ptr; 
    UREQ_BUFFER* apstUReqBase = null_ptr;
    UREQ_BUFF_INTERNAL* apstUReqData = null_ptr;
    
    if ( NULL == pstMasterArchive || NULL == pvUReqBuffer )
    {
        set_error(pstMasterArchive, ERR_INVALID_PARAM);
        print_err("%s:<KO-ADD><GA[%p]OB[%p]>", __FUNCTION__, pstMasterArchive, pvUReqBuffer);
        DBG_EXIT
        return;
    }

    if ( get_error( pstMasterArchive ) != ERR_OK )
    {
        print_err("%s:ErrorInPreviousState<KO><%d>", __FUNCTION__, get_error( pstMasterArchive ));
        DBG_EXIT
        return;
    }


    /* Pre-condition to evaluate the Sequence number */
    if ( ((UREQ_METADATA*)pvUReqBuffer)->u32RequestSequenceNumber <= pstMasterArchive->u32CurrSequence )
    {
        print_dbg("%s:RequestSequenceNumber<KO><OB[%p]SN[%d]CN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA*)pvUReqBuffer)->u32RequestSequenceNumber, pstMasterArchive->u32CurrSequence);
        set_error(pstMasterArchive, ERR_STALE_ORDER);
        DBG_EXIT
        return;
    }

    // 
    if ( (((UREQ_METADATA*)pvUReqBuffer)->u32NumberOfRequests < 1 ) ||\
         (((UREQ_METADATA*)pvUReqBuffer)->u32NumberOfRequests >= MAX_USER_REQUESTS) )
    {
        print_dbg("%s:Requests over/under flow<KO><OB[%p]SN[%d]CN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA*)pvUReqBuffer)->u32NumberOfRequests, pstMasterArchive->u32CurrSequence);
        set_error(pstMasterArchive, ERR_ORDER_OOB);
    } else {
        apstUReqData = malloc ( sizeof(UREQ_BUFF_INTERNAL) * ((UREQ_METADATA*)pvUReqBuffer)->u32NumberOfRequests );

        if ( null_ptr == apstUReqData )
        {
            set_error(pstMasterArchive, ERR_MEMORY_ALLOCATION_FAIL);
            print_dbg("%s:MemoryAllocation<KO><NO[%d]CS[%d]>", __FUNCTION__, ((UREQ_METADATA*)pvUReqBuffer)->u32NumberOfRequests, pstMasterArchive->u32CurrSequence);

        } else {
            
            UINT32 u32RequestsLooper = 0;
            UREQ_BUFF_INTERNAL stPopulationTemp = { 0 };
            apstUReqBase = ((UREQ_METADATA*)pvUReqBuffer) + 1; 
            
            
            while ( u32RequestsLooper < ((UREQ_METADATA*)pvUReqBuffer)->u32NumberOfRequests )
            {
                apstCurrUReqData = apstUReqBase + u32RequestsLooper;
                // Validate each requests and populate the global archive
                if ( populate_request( apstCurrUReqData, &stPopulationTemp ) == ERR_OK )
                {
                    memcpy( (apstUReqData + u32RequestsLooper), &stPopulationTemp, sizeof(UREQ_BUFF_INTERNAL) );
                    print_dbg("%s:RequestParsed<OK><OB[%p]SN[%d]CN[%d]RN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA*)pvUReqBuffer)->u32RequestSequenceNumber, pstMasterArchive->u32CurrSequence, u32RequestsLooper);
                } else {
                    print_err("%s:RequestValidation<KO><OB[%p]SN[%d]CN[%d]RN[%d]>", __FUNCTION__, pvUReqBuffer, ((UREQ_METADATA*)pvUReqBuffer)->u32RequestSequenceNumber, pstMasterArchive->u32CurrSequence, u32RequestsLooper);
                    set_error(pstMasterArchive, ERR_INVALID_REQUEST);
                }
                
                u32RequestsLooper++;
            }

            pstMasterArchive->pvRequests = apstUReqData;
            pstMasterArchive->u16NumberOfRequests = ((UREQ_METADATA*)pvUReqBuffer)->u32NumberOfRequests;
            pstMasterArchive->u32CurrSequence = ((UREQ_METADATA*)pvUReqBuffer)->u32RequestSequenceNumber;
            
            print_dbg("%s:MasterArchiveUpdated<OK><GA[%p]SN[%d]CN[%d]NR[%d]>", 
                            __FUNCTION__, 
                            pstMasterArchive, 
                            ((UREQ_METADATA*)pvUReqBuffer)->u32RequestSequenceNumber, 
                            pstMasterArchive->u32CurrSequence, 
                            pstMasterArchive->u16NumberOfRequests); 
        }
    }

    DBG_EXIT
    return;    
}

ERROR_CODE populate_request (UREQ_BUFFER* pstSRCUReqData, UREQ_BUFF_INTERNAL* pstDestUReqData)
{

    DBG_ENTRY
    
    if ( NULL == pstSRCUReqData || NULL == pstDestUReqData )
    {
        print_err("%s:RequestBuffer<KO><SRC[%p]DEST[%p]>", __FUNCTION__, pstSRCUReqData, pstDestUReqData);
        DBG_EXIT
        return ERR_INVALID_PARAM;
    }

    // Valve ID validation
    if ( pstSRCUReqData->u32ValveID < 1 || pstSRCUReqData->u32ValveID > MAX_VALVE_ID )
    {
        print_err("%s:ValveID<KO><VID[%d]MAX[%d]>", __FUNCTION__, pstSRCUReqData->u32ValveID, MAX_VALVE_ID);
        DBG_EXIT
        return ERR_INVALID_REQUEST;
    } else {
        pstDestUReqData->u32ValveID = pstSRCUReqData->u32ValveID;
    }


    // Request and timer validation
    if ( pstSRCUReqData->u8RequestType == REQUEST_VALVE_OPEN )
    {
        pstDestUReqData->u8RequestType = REQUEST_VALVE_OPEN;
        pstDestUReqData->u32TimerDuration = 0; // No timer for open request
    } else if ( pstSRCUReqData->u8RequestType == REQUEST_VALVE_CLOSE )
    {
        pstDestUReqData->u8RequestType = REQUEST_VALVE_CLOSE;
        pstDestUReqData->u32TimerDuration = 0; // No timer for close request
    } else if ( pstSRCUReqData->u8RequestType == REQUEST_VALVE_OPEN_TIMED_CLOSE || pstSRCUReqData->u8RequestType == REQUEST_VALVE_CLOSE_TIMED_OPEN )
    {
        if ( pstSRCUReqData->u32TimerDuration < MIN_TIMER_DURATION || pstSRCUReqData->u32TimerDuration > MAX_TIMER_DURATION )
        {
            print_err("%s:TimerDuration<KO><TD[%d]MIN[%d]MAX[%d]>", __FUNCTION__, pstSRCUReqData->u32TimerDuration, MIN_TIMER_DURATION, MAX_TIMER_DURATION);
            DBG_EXIT
            return ERR_INVALID_REQUEST;
        } 
        
        pstDestUReqData->u8RequestType = pstSRCUReqData->u8RequestType;
        pstDestUReqData->u32TimerDuration = pstSRCUReqData->u32TimerDuration;
    } else if ( pstSRCUReqData->u8RequestType == REQUEST_VALVE_STATUS )
    {
        pstDestUReqData->u8RequestType = REQUEST_VALVE_STATUS;
        pstDestUReqData->u32TimerDuration = 0; // No timer for status request
    }

    print_dbg("%s:RequestValidated<OK><VID[%d]RT[%d]TD[%d]>", 
                                    __FUNCTION__, 
                                    pstDestUReqData->u32ValveID, 
                                    pstDestUReqData->u8RequestType, 
                                    pstDestUReqData->u32TimerDuration);

    return ERR_OK;
}