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

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>

#include <stdint.h>
#include <pthread.h>

#include "cspscrb.h"


#define SPSCRB_CAPACITY     (1 << 20) /*1MB*/

static size_t __cspscrb_set_msg(char *msg, size_t size, uint64_t num)
{
    size_t pos;
    size_t len;
    char   data[ 16 ];

    len = snprintf(data, sizeof(data)/sizeof(data[0]), "%ld ", num);

    for(pos = 0; pos + len <= size; pos += len)
    {
        memcpy(msg + pos, data, len);
    }

#if 0
    if(pos < size)
    {
        memcpy(msg + pos, data, size - pos);
        pos = size;
    }
#endif
    return (pos);
}

static uint64_t __cspscrb_check_msg(char *msg, size_t size)
{
    uint64_t    num;
    size_t      pos;
    size_t      len;

    num = 0;

    for(len = 0; len < size; len ++)
    {
        if('0' > msg[ len ] || '9' < msg[ len ])
        {
            break;
        }

        num = num * 10 + msg[ len ] - '0';
    }

    assert(0 < len);

    for(pos = 0; pos < size; pos ++)
    {
        assert(msg[ pos ] == msg[ pos % (len + 1) ]);
    }

    fprintf(stdout, "[CHECK] %.*s => %ld\n", 16, msg, num);
    fflush(stdout);

    return (num);
}

void *cspscrb_thread_producer(void *cspscrb)
{
    uint64_t        idx;
    char            msg[1 * 1024];

    static __thread uint64_t    produced = 0;

    assert(CSPSCRB_MSG_MAX_LENGTH(SPSCRB_CAPACITY) >= sizeof(msg)/sizeof(msg[0]));

    for(idx = 1; idx < 1000 * 10000;)
    {
        size_t      len;
        CSPSC_BOOL  ret;

        len = __cspscrb_set_msg(msg, sizeof(msg)/sizeof(msg[0]), idx);

        ret = cspscrb_write((volatile CSPSCRB *)cspscrb, (void *)msg, len);

        if(CSPSC_OFFER_SUCC == ret)
        {
            produced += len;

            fprintf(stdout, "[PRODUCER] offer %ld => produced %ld\n", idx, produced);
            fflush(stdout);

            idx ++;
            continue;
        }

        if(CSPSC_OFFER_ERR == ret)
        {
            fprintf(stdout, "[PRODUCER] offer %ld => produced %ld => failed\n", idx, produced);
            fflush(stdout);
            break;
        }

        /*full => write again*/
    }
    return (NULL);
}

void cspscrb_consumer_do(char *msg, size_t size)
{
    uint64_t    idx;
    static __thread uint64_t    marker = 0;

    idx = __cspscrb_check_msg(msg, size);

    assert(marker + 1 == idx);

    fprintf(stdout, "[CONSUMER] consumer %ld\n", idx);
    fflush(stdout);

    marker ++;

    return;
}

void *cspscrb_thread_consumer(void *cspscrb)
{
    for(;;)
    {
        uint64_t    consumed;

        consumed = cspscrb_read((volatile CSPSCRB *)cspscrb, (CSPSCRB_DRAIN_FUNC)cspscrb_consumer_do, 128);
        if(0 < consumed)
        {
            fprintf(stdout, "[CONSUMER] ------------------ consumed %ld\n", consumed);
        }
    }

    return (NULL);
}

int main(int argc, char **argv)
{
    CSPSCRB          cspscrb;

    pthread_attr_t  attribute;
    pthread_t       producer_tid;
    pthread_t       consumer_tid;

    pthread_attr_init(&attribute);
    pthread_attr_setinheritsched(&attribute, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attribute, SCHED_OTHER);

    pthread_attr_setscope(&attribute, PTHREAD_SCOPE_PROCESS);
    pthread_attr_setstacksize(&attribute, 64 * 1024);
    pthread_attr_setguardsize(&attribute,  4 * 1024);

    cspscrb_init(&cspscrb, SPSCRB_CAPACITY + CSPSCRB_TRAILER_LENGTH);

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
    assert(0 == pthread_create(&consumer_tid, &attribute, cspscrb_thread_consumer, (void *)&cspscrb));

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
    assert(0 == pthread_create(&producer_tid, &attribute, cspscrb_thread_producer, (void *)&cspscrb));

    pthread_join(consumer_tid, NULL);
    pthread_join(producer_tid, NULL);

    cspscrb_clean(&cspscrb);

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

