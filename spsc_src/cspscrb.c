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

#include "cspscrb.h"
#include "catomic.h"

void *cspscrb_alloc(size_t size)
{
    void *ptr;

    ptr = malloc(size);

    if(NULL == ptr)
    {
        errno = ENOMEM;
        return (NULL);
    }

    memset(ptr, 0, size);

    return (ptr);
}

void cspscrb_free(void *ptr)
{
    free(ptr);
    return;
}

CSPSC_BOOL cspscrb_init(volatile CSPSCRB *cspscrb, size_t length)
{
    const size_t capacity = length - CSPSCRB_TRAILER_LENGTH;

    if (C_IS_POWER_OF_TWO(capacity))
    {
        void *buffer;

        buffer = cspscrb_alloc(length);
        if(!buffer)
        {
            return (CSPSC_FALSE);
        }

        cspscrb->buffer             = buffer;
        cspscrb->capacity           = capacity;
        cspscrb->descriptor         = (CSPSCRB_DESC *)(cspscrb->buffer + cspscrb->capacity);
        cspscrb->max_message_length = CSPSCRB_MSG_MAX_LENGTH(cspscrb->capacity);

        return (CSPSC_TRUE);
    }

    return (CSPSC_FALSE);
}

CSPSC_BOOL cspscrb_clean(CSPSCRB *cspscrb)
{
    if(cspscrb->buffer)
    {
        cspscrb_free(cspscrb->buffer);
        cspscrb->buffer     = NULL;
        cspscrb->descriptor = NULL;
    }
    return (CSPSC_TRUE);
}

CSPSC_BOOL cspscrb_write(volatile CSPSCRB *cspscrb, const void *msg, size_t length)
{
    const size_t record_length     = length + CSPSCRB_REC_HEADER_LENGTH;
    const size_t required_capacity = C_ALIGN(record_length, CSPSCRB_ALIGNMENT);
    const size_t mask              = cspscrb->capacity - 1;/*capacity is 2^n*/

    int64_t head = cspscrb->descriptor->head_cache_position;
    int64_t tail = cspscrb->descriptor->tail_position;
    const int32_t available_capacity = (int32_t)cspscrb->capacity - (int32_t)(tail - head);

    size_t padding                    = 0;
    size_t record_index               = ((size_t)tail) & mask;
    const size_t to_buffer_end_length = cspscrb->capacity - record_index;
    CSPSCRB_REC_DESC *record_header   = NULL;

    if (length > cspscrb->max_message_length)
    {
        return (CSPSC_OFFER_ERR);
    }

    if ((int32_t)required_capacity > available_capacity)
    {
        C_GET_VOLATILE(head, cspscrb->descriptor->head_position);

        if (required_capacity > (cspscrb->capacity - (size_t)(tail - head)))
        {
            return (CSPSC_OFFER_FULL);
        }

        cspscrb->descriptor->head_cache_position = head;
    }

    if (required_capacity > to_buffer_end_length)
    {
        size_t head_index = ((int32_t)head) & mask;

        if (required_capacity > head_index)
        {
            C_GET_VOLATILE(head, cspscrb->descriptor->head_position);
            head_index = ((int32_t)head) & mask;

            if (required_capacity > head_index)
            {
                return (CSPSC_OFFER_FULL);
            }

            C_PUT_ORDERED(cspscrb->descriptor->head_cache_position, head);
        }

        padding = to_buffer_end_length;
    }

    if (0 != padding)
    {
        record_header = (CSPSCRB_REC_DESC *)(cspscrb->buffer + record_index);

        record_header->status = CSPSCRB_REC_STATUS_IS_WRITING;
        C_PUT_ORDERED(record_header->length, padding);
        record_index = 0;
    }

    record_header = (CSPSCRB_REC_DESC *)(cspscrb->buffer + record_index);
    memcpy(cspscrb->buffer + CSPSCRB_MSG_OFFSET(record_index), msg, length);
    record_header->status = CSPSCRB_REC_STATUS_IS_READY;
    C_PUT_ORDERED(record_header->length, record_length);
    C_PUT_ORDERED(cspscrb->descriptor->tail_position, tail + required_capacity + padding);

    return (CSPSC_OFFER_SUCC);
}

size_t cspscrb_read( volatile CSPSCRB *cspscrb, CSPSCRB_DRAIN_FUNC handler, size_t msg_count_limit)
{
    const int64_t head      = cspscrb->descriptor->head_position;
    const size_t head_index = ((int32_t)head) & (cspscrb->capacity - 1);
    const size_t max_length = cspscrb->capacity - head_index;
    size_t msg_drain_num    = 0;
    size_t drain_nbytes     = 0;

    while ((drain_nbytes < max_length) && (msg_drain_num < msg_count_limit))
    {
        CSPSCRB_REC_DESC *header  = NULL;
        const size_t record_index = head_index + drain_nbytes;
        int32_t record_length     = 0;
        int32_t status            = 0;

        header = (CSPSCRB_REC_DESC *)(cspscrb->buffer + record_index);
        C_GET_VOLATILE(record_length, header->length);

        if (record_length <= 0)
        {
            break;
        }

        drain_nbytes += C_ALIGN(record_length, CSPSCRB_ALIGNMENT);
        status = header->status;

        if (CSPSCRB_REC_STATUS_NOT_USED == status)
        {
            break;
        }

        if (CSPSCRB_REC_STATUS_IS_WRITING == status)
        {
            continue;
        }

        ++ msg_drain_num;

        handler(cspscrb->buffer + CSPSCRB_MSG_OFFSET(record_index),
                record_length - CSPSCRB_REC_HEADER_LENGTH);

        C_PUT_VOLATILE(header->length, 0); /*xxx*/
        C_PUT_VOLATILE(header->status, CSPSCRB_REC_STATUS_NOT_USED); /*xxx*/
    }

    if (0 != drain_nbytes)
    {
        memset(cspscrb->buffer + head_index, 0, drain_nbytes);
        C_PUT_ORDERED(cspscrb->descriptor->head_position, head + drain_nbytes);
    }

    return (msg_drain_num);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
