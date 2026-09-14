/* *****************************************************************************
 * @file        gen.h
 * @brief       Shared types, error codes, archive structures, and logging APIs.
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
#ifndef GEN_H
#define GEN_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

/*============================================================================*/
/*                         DEFINES & MACROS & ENUMS                           */
/*============================================================================*/

/*============================================================================*/
/*                            TYPE DEFINITIONS                                */
/*============================================================================*/

/* ================== TYPEDEFS ================== */
typedef int INT32;
typedef unsigned int UINT32;
typedef short INT16;
typedef unsigned short UINT16;
typedef char INT8;
typedef unsigned char UINT8;
typedef void *PVOID;
typedef char *PCHAR;
typedef char CHAR;

/** Error codes returned by master operations. */
typedef enum {
    ERR_OK = 0,                         /**< Operation completed successfully. */
    ERR_NET_IF_FAIL,                    /**< Network interface failure. */
    ERR_INVALID_PARAM,                  /**< Function argument is invalid. */
    ERR_MEMORY_ALLOCATION_FAIL,         /**< Dynamic allocation failed. */
    ERR_INTEGRITY_CHECK_FAILED,         /**< Buffer integrity check failed. */
    ERR_INVALID_REQUEST,                /**< Request contents are invalid. */
    ERR_TIMEOUT,                        /**< Operation timed out. */
    ERR_STALE_ORDER,                    /**< Order sequence is stale. */
    ERR_ORDER_OOB,                      /**< Request count is out of bounds. */
    ERR_UNKNOWN                         /**< Unclassified error. */
} ERROR_CODE;

/** Runtime state of the master controller. */
typedef enum {
    MASTER_IDLE = 0,                    /**< Master is waiting for work. */
    MASTER_PROCESSING                   /**< Master is processing a request. */
} MASTER_STATE;

#define DBG_ENTRY print_dbg("%s:Entry", __FUNCTION__);
#define DBG_EXIT print_dbg("%s:Exit", __FUNCTION__);

/* ================== STRUCTURES ================== */
/**
 * @typedef MASTER_ARCHIVE
 * @brief Persistent state and parsed requests owned by the master controller.
 */
typedef struct {
    ERROR_CODE enInstErrStatus;         /**< Current instance error status. */
    UINT32 u32CurrSequence;             /**< Current request sequence number. */
    UINT32 u32PrevSequence;             /**< Previously processed sequence number. */
    PVOID pvRequests;                   /**< Allocated internal request array. */
    UINT16 u16NumberOfRequests;         /**< Number of entries in the request array. */
    UINT16 u16PositiveAckCount;         /**< Number of successful acknowledgments. */
    MASTER_STATE enMasterState;         /**< Current controller state. */
} MASTER_ARCHIVE;

/* ================== FUNCTION PROTOTYPES ================== */
/** Writes a debug-level formatted message when debugging is enabled. */
static inline void print_dbg(const char *fmt, ...);

/** Writes an error-level formatted message when error logging is enabled. */
static inline void print_err(const char *fmt, ...);

/** Writes an informational formatted message when info logging is enabled. */
static inline void print_info(const char *fmt, ...);

/** Stores an error code in a master archive when the archive is valid. */
static inline void set_error(MASTER_ARCHIVE *pstMasterArchive, ERROR_CODE enCurrentErr);

/** Reads the archive error code, returning ERR_INVALID_PARAM for NULL. */
static inline ERROR_CODE get_error(const MASTER_ARCHIVE *pstMasterArchive);

/** Converts an error code to its stable string representation. */
static inline const char *convert_err2str(ERROR_CODE enErrorCode);

/* ================== INLINE FUNCTIONS ================== */
static inline void print_dbg(const char *fmt, ...) {
#ifdef ENABLE_DBG
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
#endif
}

static inline void print_err(const char *fmt, ...) {
#ifdef ENABLE_ERROR
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
#endif
}

static inline void print_info(const char *fmt, ...) {
#ifdef ENABLE_INFO
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
#endif
}

static inline void set_error(MASTER_ARCHIVE *pstMasterArchive, ERROR_CODE enCurrentErr)
{
    if ( pstMasterArchive != NULL ) {
        pstMasterArchive->enInstErrStatus = enCurrentErr;
    }
}

static inline ERROR_CODE get_error(const MASTER_ARCHIVE *pstMasterArchive)
{
    return (pstMasterArchive != NULL) ? pstMasterArchive->enInstErrStatus : ERR_INVALID_PARAM;
}

static inline const char *convert_err2str(ERROR_CODE enErrorCode)
{
    switch (enErrorCode) {
        case ERR_OK: return "ERR_OK";
        case ERR_NET_IF_FAIL: return "ERR_NET_IF_FAIL";
        case ERR_INVALID_PARAM: return "ERR_INVALID_PARAM";
        case ERR_MEMORY_ALLOCATION_FAIL: return "ERR_MEMORY_ALLOCATION_FAIL";
        case ERR_INTEGRITY_CHECK_FAILED: return "ERR_INTEGRITY_CHECK_FAILED";
        case ERR_INVALID_REQUEST: return "ERR_INVALID_REQUEST";
        case ERR_TIMEOUT: return "ERR_TIMEOUT";
        case ERR_STALE_ORDER: return "ERR_STALE_ORDER";
        case ERR_ORDER_OOB: return "ERR_ORDER_OOB";
        default: return "ERR_UNKNOWN";
    }
}

#endif /* GEN_H */
