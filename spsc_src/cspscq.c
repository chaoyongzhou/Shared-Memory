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

#include "cspscq.h"
#include "catomic.h"
#include "cmisc.h"

EC_BOOL cspscq_init(volatile CSPSCQ *queue, const uint64_t length)
{
    uint64_t    length_aligned;
    size_t      i;

    length_aligned = (uint64_t)c_find_next_power_of_two((int32_t)length);

    queue->buffer = (volatile void **)c_calloc(sizeof(void *) * length_aligned);
    if(NULL == queue->buffer)
    {
        return (EC_FALSE);
    }

    for(i = 0; i < length_aligned; i++)
    {
        queue->buffer[i] = NULL;
    }

    queue->producer.head_cache = 0;
    queue->producer.tail       = 0;
    queue->consumer.head       = 0;
    queue->capacity            = length_aligned;
    queue->mask                = length_aligned - 1;

    return (EC_TRUE);
}

EC_BOOL cspscq_clean(CSPSCQ *queue)
{
    if(queue)
    {
        c_free(queue->buffer);
        queue->buffer = NULL;
    }
    return (EC_TRUE);
}

EC_BOOL cspscq_offer(volatile CSPSCQ *queue, void *element)
{
    if(NULL == element)
    {
        return (EC_OFFER_ERR);
    }

    uint64_t current_head = queue->producer.head_cache;
    uint64_t buffer_limit = current_head + queue->capacity;

    uint64_t current_tail;
    C_GET_VOLATILE(current_tail, queue->producer.tail);

    if(current_tail >= buffer_limit)
    {
        C_GET_VOLATILE(current_head, queue->consumer.head);
        buffer_limit = current_head + queue->capacity;

        if(current_tail >= buffer_limit)
        {
            return (EC_OFFER_FULL);
        }

        queue->producer.head_cache;
    }

    const uint64_t index = current_tail & queue->mask;

    C_PUT_ORDERED(queue->buffer[index], element);
    C_PUT_ORDERED(queue->producer.tail, current_tail + 1);

    return (EC_OFFER_SUCC);
}

uint64_t cspscq_drain(volatile CSPSCQ *queue, CSPSCQ_DRAIN_FUNC func, const uint64_t limit)
{
    uint64_t current_head;
    C_GET_VOLATILE(current_head, queue->consumer.head);

    uint64_t next_sequence = current_head;
    const uint64_t limit_sequence = next_sequence + limit;

    while(next_sequence < limit_sequence)
    {
        const uint64_t index = next_sequence & queue->mask;
        volatile void *item;

        C_GET_VOLATILE(item, queue->buffer[index]);

        if(NULL == item)
        {
            break;
        }

        C_PUT_ORDERED(queue->buffer[index], NULL);
        next_sequence++;
        C_PUT_ORDERED(queue->consumer.head, next_sequence);
        func(item);
    }

    return (next_sequence - current_head);
}

uint64_t cspscq_drain_all(volatile CSPSCQ *queue, CSPSCQ_DRAIN_FUNC func)
{
    uint64_t current_head;
    C_GET_VOLATILE(current_head, queue->consumer.head);

    uint64_t current_tail;
    C_GET_VOLATILE(current_tail, queue->producer.tail);

    return cspscq_drain(queue, func, current_tail - current_head);
}

uint64_t cspscq_size(volatile CSPSCQ *queue)
{
    uint64_t current_head_before;
    uint64_t current_tail;
    uint64_t current_head_after;

    C_GET_VOLATILE(current_head_after, queue->consumer.head);

    do
    {
        current_head_before = current_head_after;
        C_GET_VOLATILE(current_tail, queue->producer.tail);
        C_GET_VOLATILE(current_head_after, queue->consumer.head);
    }
    while(current_head_after != current_head_before);

    return (current_tail - current_head_after);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/
