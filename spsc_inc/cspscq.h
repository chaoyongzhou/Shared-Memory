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

#ifndef _CSPSCQ_H
#define _CSPSCQ_H

#include "cspsc.h"


typedef void (*CSPSCQ_DRAIN_FUNC)(volatile void *);

/*spsc concurrent array queue*/
typedef struct
{
    int8_t       padding[(2 * C_CACHE_LINE_LENGTH)];

    struct
    {
        uint64_t tail;
        uint64_t head_cache;
        int8_t   padding[(2 * C_CACHE_LINE_LENGTH) - (2 * sizeof(uint64_t))];
    }producer;

    struct
    {
        uint64_t head;
        int8_t   padding[(2 * C_CACHE_LINE_LENGTH) - (1 * sizeof(uint64_t))];
    }consumer;

    uint64_t        capacity;
    uint64_t        mask;
    volatile void **buffer;
}
CSPSCQ;

CSPSC_BOOL cspscq_init(volatile CSPSCQ *queue, const uint64_t length);

CSPSC_BOOL cspscq_clean(CSPSCQ *queue);

CSPSC_BOOL cspscq_offer(volatile CSPSCQ *queue, void *element);

uint64_t cspscq_drain(volatile CSPSCQ *queue, CSPSCQ_DRAIN_FUNC func, const uint64_t limit);

uint64_t cspscq_drain_all(volatile CSPSCQ *queue, CSPSCQ_DRAIN_FUNC func);

uint64_t cspscq_size(volatile CSPSCQ *queue);


#endif/*_CSPSCQ_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

