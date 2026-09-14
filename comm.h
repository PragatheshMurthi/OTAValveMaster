/* *****************************************************************************
 * @file        comm.h
 * @brief       Public request, order, acknowledgment, and CRC interfaces.
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
#ifndef __COMM__
#define __COMM__

#include "gen.h"

/*============================================================================*/
/*                         DEFINES & MACROS & ENUMS                           */
/*============================================================================*/

/** Supported valve request actions. */
enum {
    REQUEST_VALVE_OPEN = 0,             /**< Open the valve. */
    REQUEST_VALVE_CLOSE,                /**< Close the valve. */
    REQUEST_VALVE_OPEN_TIMED_CLOSE,     /**< Open, then close after a timer. */
    REQUEST_VALVE_CLOSE_TIMED_OPEN,     /**< Close, then open after a timer. */
    REQUEST_VALVE_STATUS                /**< Query the valve status. */
};

/** Processing states for an order sent to a valve node. */
enum {
    ORDER_STATUS_PROCESSING = 0,        /**< Order is being processed. */
    ORDER_STATUS_ACK_PENDING,           /**< Waiting for a valve acknowledgment. */
    ORDER_STATUS_ACK_RECEIVED,          /**< Acknowledgment was received. */
    ORDER_STATUS_FAILED                 /**< Order processing failed. */
};

/**
 * @typedef UREQ_METADATA
 * @brief Metadata prefixed to a user request payload.
 */
typedef struct {
    UINT32 u32RequestLength;            /**< Total request length in bytes. */
    UINT32 u32RequestCRC;               /**< CRC of the complete request. */
    UINT32 u32NumberOfRequests;         /**< Number of request entries. */
    UINT32 u32RequestSequenceNumber;    /**< Sequence number assigned by the user. */
} UREQ_METADATA;

/**
 * @typedef UREQ_BUFFER
 * @brief Single valve request received from a user.
 */
typedef struct {
    UINT32 u32ValveID;                  /**< Target valve identifier. */
    UINT32 u8RequestType;               /**< Value from the request action list. */
    UINT32 u32TimerDuration;            /**< Timer duration in seconds. */
} UREQ_BUFFER;

/**
 * @typedef UREQ_BUFF_INTERNAL
 * @brief Internal request representation used by the master.
 */
typedef struct {
    UINT32 u32ValveID;                  /**< Target valve identifier. */
    UINT32 u8RequestType;               /**< Validated request action. */
    UINT32 u32TimerDuration;            /**< Validated timer duration in seconds. */
    UINT32 u8OrderStatus;               /**< Current order processing state. */
    CHAR acOrderResult[MAX_ORDER_RESULT_LENGTH]; /**< Result text for the request. */
} UREQ_BUFF_INTERNAL;

/**
 * @typedef SLAVE_ORDER_BUFFER
 * @brief Wire-format order sent from the master to a valve node.
 */
typedef struct {
    UINT32 u32OrderLength;              /**< Total order length in bytes. */
    UINT32 u32OrderCRC;                 /**< CRC of the complete order. */
    UINT32 u32ValveNumber;              /**< Target valve identifier. */
    UINT32 u32OrderSequenceNumber;      /**< Master order sequence number. */
    UINT8 u8ActionType;                 /**< Action requested for the valve. */
    UINT8 u8Reserved[3];                /**< Reserved alignment bytes. */
    UINT32 u32TimerCntS;                /**< Timer duration in seconds. */
} SLAVE_ORDER_BUFFER;

/**
 * @typedef ACK_BUFFER
 * @brief Wire-format acknowledgment received from a valve node.
 */
typedef struct {
    UINT32 u32OrderLength;              /**< Total acknowledgment length in bytes. */
    UINT32 u32OrderCRC;                 /**< CRC of the complete acknowledgment. */
    UINT32 u32SequenceNumber;           /**< Sequence number acknowledged. */
    UINT32 u32ValveID;                  /**< Valve identifier returning the acknowledgment. */
    CHAR acOrderStatus[MAX_ORDER_RESULT_LENGTH]; /**< Order result text. */
} ACK_BUFFER;

/**
 * @typedef USR_ACK_BUFFER
 * @brief Aggregated acknowledgment returned to the requesting user.
 */
typedef struct {
    UINT32 u32OrderLength;              /**< Total acknowledgment length in bytes. */
    UINT32 u32OrderCRC;                 /**< CRC of the complete acknowledgment. */
    UINT32 u32SequenceNumber;           /**< Sequence number being acknowledged. */
    UINT32 u32RequestState;             /**< Aggregated request state. */
    CHAR acRequestStatus[MAX_USR_ACK_RESULT_LENGTH]; /**< Request result text. */
} USR_ACK_BUFFER;

/** Receives and integrity-checks one user request.
 *  @param[in] pstMasterArchive Master state used for error reporting.
 *  @return Allocated request buffer, or NULL when reception fails.
 */
PVOID receive_user_request(MASTER_ARCHIVE *pstMasterArchive);

/** Parses a received request into the master archive.
 *  @param[in,out] pstMasterArchive Master state to update.
 *  @param[in] pvUReqBuffer Received user request buffer.
 */
void parse_request(MASTER_ARCHIVE *pstMasterArchive, PVOID pvUReqBuffer);

/** Validates and copies one user request into internal storage.
 *  @param[in] pstSRCUReqData Source request data.
 *  @param[out] pstDestUReqData Validated internal request data.
 *  @return ERR_OK on success, otherwise the validation error.
 */
ERROR_CODE populate_request(UREQ_BUFFER *pstSRCUReqData, UREQ_BUFF_INTERNAL *pstDestUReqData);

/** Verifies the CRC embedded in a user request buffer.
 *  @param[in,out] pvUReqBuffer Request buffer whose CRC is checked.
 *  @return ERR_OK when the CRC matches, otherwise an error code.
 */
ERROR_CODE evaluate_integrity(PVOID pvUReqBuffer);

/** Calculates the CRC for a byte buffer.
 *  @param[in] pvOrderBuffer Buffer to checksum.
 *  @param[in] u16OrderLength Buffer length in bytes.
 *  @return Calculated CRC value, or zero for invalid input.
 */
UINT32 calculate_crc(const void *pvOrderBuffer, UINT16 u16OrderLength);

/** Finds an internal request by valve identifier.
 *  @param[in] pstMasterArchive Master archive containing requests.
 *  @param[in] u32ValveID Valve identifier to find.
 *  @return Matching request, or NULL when no match exists.
 */
UREQ_BUFF_INTERNAL *find_instance_by_valve_id(MASTER_ARCHIVE *pstMasterArchive, UINT32 u32ValveID);

/** Sends all parsed orders and processes their valve acknowledgments.
 *  @param[in,out] pstMasterArchive Master archive containing the orders.
 */
void initiate_order(MASTER_ARCHIVE *pstMasterArchive);

/** Builds and sends the aggregated user acknowledgment.
 *  @param[in,out] pstMasterArchive Master archive containing order results.
 */
void post_ack(MASTER_ARCHIVE *pstMasterArchive);

/** Normalizes the status text in an acknowledgment buffer.
 *  @param[out] pchAckStatus Acknowledgment status text.
 *  @param[in] u16AckLength Status buffer length.
 *  @param[in] pstMasterArchive Master archive supplying sequence context.
 *  @return ERR_OK on success, otherwise an invalid-parameter error.
 */
ERROR_CODE fill_ack_buffer(CHAR *pchAckStatus, UINT16 u16AckLength, MASTER_ARCHIVE *pstMasterArchive);

#endif /* __COMM__ */