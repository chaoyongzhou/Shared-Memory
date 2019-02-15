/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@163.com
* QQ: 2796796
*
*******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#ifndef _CSPSCRB_H
#define _CSPSCRB_H

#include "type.h"
#include "cbitutil.h"

#define CSPSCRB_REC_STATUS_NOT_USED     ((int32_t) 0)
#define CSPSCRB_REC_STATUS_IS_WRITING   ((int32_t) 1)
#define CSPSCRB_REC_STATUS_IS_READY     ((int32_t) 2)

typedef struct
{
    uint8_t begin_pad[(2 * C_CACHE_LINE_LENGTH)];

    int64_t tail_position;
    uint8_t tail_pad[(2 * C_CACHE_LINE_LENGTH) - sizeof(int64_t)];

    int64_t head_cache_position;
    uint8_t head_cache_pad[(2 * C_CACHE_LINE_LENGTH) - sizeof(int64_t)];

    int64_t head_position;
    uint8_t head_pad[(2 * C_CACHE_LINE_LENGTH) - sizeof(int64_t)];
}CSPSCRB_DESC;

#define CSPSCRB_TRAILER_LENGTH (sizeof(CSPSCRB_DESC))

typedef struct
{
    int32_t length;
    int32_t status;
}CSPSCRB_REC_DESC;

#define CSPSCRB_REC_HEADER_LENGTH             (sizeof(CSPSCRB_REC_DESC))


typedef struct
{
    uint8_t          *buffer;
    CSPSCRB_DESC     *descriptor;
    size_t            capacity;
    size_t            max_message_length;
}CSPSCRB;

#define CSPSCRB_ALIGNMENT                     (2 * sizeof(int32_t))

#define CSPSCRB_MSG_OFFSET(index)             (index + sizeof(CSPSCRB_REC_DESC))

#define CSPSCRB_MSG_MAX_LENGTH(capacity)      (capacity / 8)

typedef void (*CSPSCRB_DRAIN_FUNC)(const void *, size_t);

EC_BOOL cspscrb_init(volatile CSPSCRB *cspscrb, size_t length);

EC_BOOL cspscrb_clean(CSPSCRB *cspscrb);

EC_BOOL cspscrb_write( volatile CSPSCRB *cspscrb, const void *msg, size_t length);

size_t cspscrb_read( volatile CSPSCRB *cspscrb, CSPSCRB_DRAIN_FUNC handler, size_t msg_count_limit);

#endif/*_CSPSCRB_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/


