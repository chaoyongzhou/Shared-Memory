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

#include "cmpscrb.h"
#include "catomic.h"
#include "cmisc.h"

EC_BOOL cmpscrb_init(volatile CMPSCRB *cmpscrb, size_t length)
{
    const size_t capacity = length - CMPSCRB_TRAILER_LENGTH;

    if(C_IS_POWER_OF_TWO(capacity))
    {
        void *buffer;

        buffer = c_calloc(length);
        if(!buffer)
        {
            return (EC_FALSE);
        }

        cmpscrb->buffer             = buffer;
        cmpscrb->capacity           = capacity;
        cmpscrb->descriptor         = (CMPSCRB_DESC *)(cmpscrb->buffer + cmpscrb->capacity);
        cmpscrb->max_message_length = CMPSCRB_MESSAGE_MAX_LENGTH(cmpscrb->capacity);

        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmpscrb_clean(CMPSCRB *cmpscrb)
{
    if(cmpscrb->buffer)
    {
        c_free(cmpscrb->buffer);
        cmpscrb->buffer     = NULL;
        cmpscrb->descriptor = NULL;
    }
    return (EC_TRUE);
}

static int32_t __cmpscrb_claim_capacity(volatile CMPSCRB *cmpscrb, size_t required_capacity)
{
    const size_t mask                   = cmpscrb->capacity - 1;
    int64_t      head                   = 0;
    int64_t      tail                   = 0;
    size_t       tail_index             = 0;
    size_t       padding                = 0;
    size_t       to_buffer_end_length   = 0;

    C_GET_VOLATILE(head, cmpscrb->descriptor->head_cache_position);

    do
    {
        int32_t available_capacity = 0;
        C_GET_VOLATILE(tail, cmpscrb->descriptor->tail_position);

        padding = 0; /*reset*/

        available_capacity = (int32_t)cmpscrb->capacity - (int32_t)(tail - head);

        if((int32_t)required_capacity > available_capacity)
        {
            C_GET_VOLATILE(head, cmpscrb->descriptor->head_position);

            available_capacity = (int32_t)cmpscrb->capacity - (int32_t)(tail - head);

            if((int32_t)required_capacity > available_capacity)
            {
                return (-1);
            }

            C_PUT_ORDERED(cmpscrb->descriptor->head_cache_position, head);
        }

        tail_index = (int32_t)tail & mask;
        to_buffer_end_length = cmpscrb->capacity - tail_index;

        if(required_capacity > to_buffer_end_length)
        {
            size_t head_index = (int32_t)head & mask;

            if(required_capacity > head_index)
            {
                C_GET_VOLATILE(head, cmpscrb->descriptor->head_position);
                head_index = (int32_t)head & mask;

                if(required_capacity > head_index)
                {
                    return (-1);
                }

                C_PUT_ORDERED(cmpscrb->descriptor->head_cache_position, head);
            }

            padding = to_buffer_end_length;
        }
    } while(!c_cmpxchg64(&(cmpscrb->descriptor->tail_position),
                        tail,
                        tail + (int32_t)required_capacity + (int32_t)padding));

    if(0 != padding)
    {
        CMPSCRB_REC_DESC *record_header = (CMPSCRB_REC_DESC *)(cmpscrb->buffer + tail_index);

        record_header->status = CMPSCRB_REC_STATUS_NOT_USED;
        C_PUT_ORDERED(record_header->length, padding);

        tail_index = 0;
    }

    return ((int32_t)tail_index);
}

EC_BOOL cmpscrb_write(volatile CMPSCRB *cmpscrb, const void *msg, size_t length)
{
    const size_t record_length      = length + CMPSCRB_RECORD_HEADER_LENGTH;
    const size_t required_capacity  = C_ALIGN(record_length, CMPSCRB_ALIGNMENT);
    int32_t      record_index       = 0;

    if(length > cmpscrb->max_message_length)
    {
        return (EC_OFFER_ERR);
    }

    record_index = __cmpscrb_claim_capacity(cmpscrb, required_capacity);

    if(-1 != record_index)
    {
        CMPSCRB_REC_DESC *record_header = (CMPSCRB_REC_DESC *)(cmpscrb->buffer + record_index);

        C_PUT_ORDERED(record_header->length, (int32_t)-record_length);

        memcpy(cmpscrb->buffer + CMPSCRB_MESSAGE_OFFSET(record_index), msg, length);

        record_header->status = CMPSCRB_REC_STATUS_IS_READY;
        C_PUT_ORDERED(record_header->length, (int32_t)record_length);
        return (EC_OFFER_SUCC);
    }

    return (EC_OFFER_FULL);
}

size_t cmpscrb_read(volatile CMPSCRB *cmpscrb, CMPSCRB_DRAIN_FUNC handler, size_t message_count_limit)
{
    const int64_t   head                          = cmpscrb->descriptor->head_position;
    const size_t    head_index                    = (int32_t)head & (cmpscrb->capacity - 1);
    const size_t    contiguous_block_length       = cmpscrb->capacity - head_index;
    size_t          messages_read                 = 0;
    size_t          bytes_read                    = 0;

    while((bytes_read < contiguous_block_length) && (messages_read < message_count_limit))
    {
        CMPSCRB_REC_DESC   *header                = NULL;
        const size_t        record_index          = head_index + bytes_read;
        int32_t             record_length         = 0;
        int32_t             status                = CMPSCRB_REC_STATUS_NOT_USED;

        header = (CMPSCRB_REC_DESC *)(cmpscrb->buffer + record_index);
        C_GET_VOLATILE(record_length, header->length);

        if(record_length <= 0)
        {
            break;
        }

        bytes_read += C_ALIGN(record_length, CMPSCRB_ALIGNMENT);
        status      = header->status;

        if(CMPSCRB_REC_STATUS_NOT_USED == status)
        {
            break;
        }

        ++ messages_read;

        handler(cmpscrb->buffer + CMPSCRB_MESSAGE_OFFSET(record_index),
                record_length - CMPSCRB_RECORD_HEADER_LENGTH);
    }

    if(0 != bytes_read)
    {
        memset(cmpscrb->buffer + head_index, 0, bytes_read);
        C_PUT_ORDERED(cmpscrb->descriptor->head_position, head + bytes_read);
    }

    return (messages_read);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

