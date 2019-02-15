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

#ifndef _CMPSCRB_H
#define _CMPSCRB_H

#include "type.h"
#include "cbitutil.h"

#define CMPSCRB_REC_STATUS_NOT_USED     ((int32_t) 0)
//#define CMPSCRB_REC_STATUS_IS_WRITING   ((int32_t) 1)
#define CMPSCRB_REC_STATUS_IS_READY     ((int32_t) 2)

typedef struct
{
    uint8_t begin_pad[(2 * C_CACHE_LINE_LENGTH)];

    int64_t tail_position;
    uint8_t tail_pad[(2 * C_CACHE_LINE_LENGTH) - sizeof(int64_t)];

    int64_t head_cache_position;
    uint8_t head_cache_pad[(2 * C_CACHE_LINE_LENGTH) - sizeof(int64_t)];

    int64_t head_position;
    uint8_t head_pad[(2 * C_CACHE_LINE_LENGTH) - sizeof(int64_t)];
}CMPSCRB_DESC;

typedef struct
{
    int32_t  length;
    int32_t  status;
}CMPSCRB_REC_DESC;

typedef struct
{
    uint8_t         *buffer;
    CMPSCRB_DESC    *descriptor;
    size_t           capacity;
    size_t           max_message_length;
}CMPSCRB;

#define CMPSCRB_TRAILER_LENGTH                  (sizeof(CMPSCRB_DESC))

#define CMPSCRB_ALIGNMENT                       (2 * sizeof(int32_t))

#define CMPSCRB_MESSAGE_OFFSET(index)           (index + sizeof(CMPSCRB_REC_DESC))

#define CMPSCRB_RECORD_HEADER_LENGTH            (sizeof(CMPSCRB_REC_DESC))

#define CMPSCRB_MESSAGE_MAX_LENGTH(capacity)    (capacity / 8)

typedef void (*CMPSCRB_DRAIN_FUNC)(const void *, size_t);

EC_BOOL cmpscrb_init(volatile CMPSCRB *cmpscrb, size_t length);

EC_BOOL cmpscrb_clean(CMPSCRB *cmpscrb);

EC_BOOL cmpscrb_write(volatile CMPSCRB *cmpscrb, const void *msg, size_t length);

size_t cmpscrb_read(volatile CMPSCRB *cmpscrb, CMPSCRB_DRAIN_FUNC handler, size_t message_count_limit);

#endif/*_CMPSCRB_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/