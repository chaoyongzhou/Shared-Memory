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

#ifndef _CMPSCQ_H
#define _CMPSCQ_H

#include "type.h"

#include "cbitutil.h"


typedef void (*CMPSCQ_DRAIN_FUNC)(volatile void *);

typedef struct
{
    int8_t padding[(2 * C_CACHE_LINE_LENGTH)];

    struct
    {
        uint64_t tail;
        uint64_t head_cache;
        uint64_t shared_head_cache;
        int8_t   padding[(2 * C_CACHE_LINE_LENGTH) - (3 * sizeof(uint64_t))];
    }producer;

    struct
    {
        uint64_t head;
        int8_t   padding[(2 * C_CACHE_LINE_LENGTH) - (1 * sizeof(uint64_t))];
    }consumer;

    uint64_t        capacity;
    uint64_t        mask;
    volatile void **buffer;
}CMPSCQ;


EC_BOOL cmpscq_init(volatile CMPSCQ *queue, const uint64_t length);

EC_BOOL cmpscq_clean(CMPSCQ *queue);

EC_BOOL cmpscq_offer(volatile CMPSCQ *queue, void *element);

uint64_t cmpscq_drain( volatile CMPSCQ *queue, CMPSCQ_DRAIN_FUNC func, const uint64_t limit);

uint64_t cmpscq_drain_all( volatile CMPSCQ *queue, CMPSCQ_DRAIN_FUNC func);

uint64_t cmpscq_size(volatile CMPSCQ *queue);

#endif/*_CMPSCQ_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

