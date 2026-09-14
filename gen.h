#ifndef GEN_H
#define GEN_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

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

typedef enum {
    ERR_OK = 0,
    ERR_NET_IF_FAIL,
    ERR_INVALID_PARAM,
    ERR_MEMORY_ALLOCATION_FAIL,
    ERR_INTEGRITY_CHECK_FAILED,
    ERR_INVALID_REQUEST,
    ERR_TIMEOUT,
    ERR_STALE_ORDER,
    ERR_ORDER_OOB,
    ERR_UNKNOWN
} ERROR_CODE;

typedef enum {
    MASTER_IDLE = 0,
    MASTER_PROCESSING
} MASTER_STATE;

#define DBG_ENTRY print_dbg("%s:Entry", __FUNCTION__);
#define DBG_EXIT print_dbg("%s:Exit", __FUNCTION__);

/* ================== STRUCTURES ================== */
typedef struct {
    ERROR_CODE enInstErrStatus;
    UINT32 u32CurrSequence;
    UINT32 u32PrevSequence;
    PVOID pvRequests;
    UINT16 u16NumberOfRequests;
    UINT16 u16PositiveAckCount;
    MASTER_STATE enMasterState;
} MASTER_ARCHIVE;

/* ================== FUNCTION PROTOTYPES ================== */
static inline void print_dbg(const char *fmt, ...);
static inline void print_err(const char *fmt, ...);
static inline void print_info(const char *fmt, ...);
static inline void set_error(MASTER_ARCHIVE *pstMasterArchive, ERROR_CODE enCurrentErr);
static inline ERROR_CODE get_error(const MASTER_ARCHIVE *pstMasterArchive);
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
