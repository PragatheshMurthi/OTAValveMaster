#ifndef GEN_H
#define GEN_H

/* ================== INCLUDES ================== */

#include "options.h"
#include "netinter.h"

/* ================== TYPEDEFS ================== */

typedef int INT32;
typedef unsigned int UINT32;
typedef short INT16;
typedef unsigned short UINT16;
typedef char INT8;
typedef unsigned char UINT8;
typedef void* PVOID;
typedef char* PCHAR;

/* ================== ENUMS ================== */

typedef enum { 
    ERR_OK = 0, 
    ERR_NET_IF_FAIL,
    ERR_INVALID_PARAM,
    ERR_MEMORY_ALLOCATION_FAIL,
    ERR_INTEGRITY_CHECK_FAILED,
    ERR_INVALID_REQUEST,
    ERR_TIMEOUT,
    ERR_UNKNOWN
} ERROR_CODE;

typedef enum { 
    MASTER_IDLE = 0,
} MASTER_STATE;

/* ================== DEFINE ================== */

#define DBG_ENTRY print_dbg("%s:Entry", __FUNCTION__);
#define DBG_EXIT print_dbg("%s:Exit", __FUNCTION__); 

/* ================== STRUCTURES ================== */

typedef struct {
    ERROR_CODE enInstErrStatus;
    UINT32 u32CurrSequence;
    PVOID pvRequests;
    UINT16 u16NumberOfRequests;
    UINT32 u32CurrSequence;
    UINT32 u32PrevSequence;
    MASTER_STATE enMasterState;
    UINT16 u16PositiveAckCount;
}MASTER_ARCHIVE;

/* ================== FUNCTION PROTOTYPES ================== */

static inline void print_dbg( const char* argv, ... );
static inline void print_err( const char* argv, ... );
static inline void print_info( const char* argv, ... );
static inline void set_error ( PVOID pvInstance, ERROR_CODE enCurrentErr );
static inline ERROR_CODE get_error ( PVOID pvInstance );
static inline PCHAR convert_err2str( ERROR_CODE enErrorCode );

/* ================== INLINE FUNCTIONS ================== */
static inline void print_dbg( const char* argv, ... ) {
    va_list args;
    int done;

    // 2. Initialize the argument list
    va_start(args, format);
    done = vprintf(format, args);
    va_end(args);

    return done;
}

static inline void print_err( const char* argv, ... ) {
    va_list args;
    int done;

    // 2. Initialize the argument list
    va_start(args, format);
    done = vprintf(format, args);
    va_end(args);

    return done;
}

static inline void print_info( const char* argv, ... ) {
    va_list args;
    int done;

    // 2. Initialize the argument list
    va_start(args, format);
    done = vprintf(format, args);
    va_end(args);

    return done;
}

static inline void set_error ( PVOID pvInstance, ERROR_CODE enCurrentErr )
{
    ( pvInstance && (MASTER_ARCHIVE*)pvInstance->enInstErrStatus == ERR_OK ) ? ((*(MASTER_ARCHIVE)pvInstance)->enInstErrStatus = enCurrentErr ) : nullptr;
    return;
}

static inline ERROR_CODE get_error ( PVOID pvInstance )
{
    return (pvInstance ? ((*(GLOBAL_ARCHIVE*)pvInstance)->enInstErrStatus ) : 0);
}

static inline PCHAR convert_err2str( ERROR_CODE enErrorCode )
{
    switch ( enErrorCode )
    {
        case ERR_OK: return "ERR_OK";
        case ERR_NET_IF_FAIL: return "ERR_NET_IF_FAIL";
        case ERR_INVALID_PARAM: return "ERR_INVALID_PARAM";
        case ERR_MEMORY_ALLOCATION_FAIL: return "ERR_MEMORY_ALLOCATION_FAIL";
        case ERR_INTEGRITY_CHECK_FAILED: return "ERR_INTEGRITY_CHECK_FAILED";
        case ERR_UNKNOWN: return "ERR_UNKNOWN";
        default: return "ERR_UNKNOWN";
    }
}
#endif /* GEN_H */
