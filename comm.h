#ifndef __COMM__
#define __COMM__

/* ================== INCLUDES ================== */
#include "gen.h"

/* ================== ENUMS ================== */
enum {
    REQUEST_VALVE_OPEN = 0,
    REQUEST_VALVE_CLOSE,
    REQUEST_VALVE_OPEN_TIMED_CLOSE,
    REQUEST_VALVE_CLOSE_TIMED_OPEN,
    REQUEST_VALVE_STATUS
} REQUEST_TYPE;

enum {
    ORDER_STATUS_PROCESSING = 0,
    ORDER_STATUS_ACK_PENDING,
    ORDER_STATUS_ACK_RECEIVED,
    ORDER_STATUS_FAILED,
} ORDER_STATUS;
/* ================== STRUCTURES ================== */
typedef struct {
    /* Request metadata */
    UINT32 u32RequestLength;
    UINT32 u32RequestCRC;
    UINT32 u32NumberOfRequests;
    UINT32 u32RequestSequenceNumber;
} UREQ_METADATA;

typedef struct {
    UINT32 u32ValveID;
    REQUEST_TYPE u8RequestType;
    UINT32 u32TimerDuration;
} UREQ_BUFFER;

typedef struct {
    UINT32 u32ValveID;
    REQUEST_TYPE u8RequestType;
    UINT32 u32TimerDuration;
    ORDER_STATUS u8OrderStatus;
    CHAR acOrderResult[MAX_ORDER_RESULT_LENGTH];
} UREQ_BUFF_INTERNAL;

typedef struct {
    /* Order metadata */
    UINT32 u32OrderLength;
    UINT32 u32OrderCRC;

    /* Data starts */
    UINT32 u32ValveNumber;
    UINT32 u32OrderSequenceNumber;
    UINT8 u8ActionType;
    UINT8 u8Reserved[3];
    UINT32 u32TimerCntS;
    
} SLAVE_ORDER_BUFFER;

typedef struct {
    /* Order metadata */
    UINT32 u32OrderLength;
    UINT32 u32OrderCRC;
    UINT32 u32SequenceNumber;

    /* Data starts */
    CHAR acOrderStatus[MAX_ORDER_RESULT_LENGTH];
    
}ACK_BUFFER;

typedef struct {
    /* Order metadata */
    UINT32 u32OrderLength;
    UINT32 u32OrderCRC;
    UINT32 u32SequenceNumber;
    UINT32 u32RequestState;

    /* Data starts */
    CHAR acRequestStatus[MAX_USR_ACK_RESULT_LENGTH];
}USR_ACK_BUFFER;

/* ================== FUNCTION PROTOTYPES ================== */

PVOID receive_user_request ( MASTER_ARCHIVE* pstMasterArchive );
VOID parse_request( MASTER_ARCHIVE* pstMasterArchive, PVOID pvUReqBuffer );
ERROR_CODE populate_request (UREQ_BUFFER* pstSRCUReqData, UREQ_BUFFER* pstDestUReqData);
ERROR_CODE evaluate_integrity( PVOID pvOrderBuffer );
UINT32 calculate_crc( PVOID pvOrderBuffer, UINT16 u16OrderLength );
UREQ_BUFF_INTERNAL* find_instance_by_valve_id( MASTER_ARCHIVE* pstMasterArchive, UINT32 u32ValveID );
VOID initiate_order( MASTER_ARCHIVE* pstMasterArchive );

#endif  /* __COMM__ */