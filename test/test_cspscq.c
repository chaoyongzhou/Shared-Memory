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

#include "cspscq.h"


#define SPSCQ_QUEUE_CAPACITY            (256)

#define SPSCQ_TEST_MAX_NUMBER           (1000 * 10000)

static __thread uint64_t    g_spscq_marker = 0;

void *cspscq_producer_thread(void *spsc_queue)
{
    uint64_t        idx;

    for(idx = 1; idx < SPSCQ_TEST_MAX_NUMBER;)
    {
        if(EC_OFFER_SUCC == cspscq_offer((volatile CSPSCQ *)spsc_queue, (void *)idx))
        {
            //fprintf(stdout, "[PRODUCER] offer %ld\n", idx);
            //fflush(stdout);

            idx ++;
        }
    }
    return (NULL);
}

void cspscq_consumer_do(volatile void *item)
{
    uint64_t    idx;

    idx = (uint64_t)item;

    assert(g_spscq_marker + 1 == idx);

    //fprintf(stdout, "[CONSUMER] consumer %ld\n", idx);
    //fflush(stdout);

    g_spscq_marker ++;

    return;
}

void *cspscq_consumer_thread(void *spsc_queue)
{
    for(;;)
    {
        uint64_t    consumed;

        consumed = cspscq_drain_all((volatile CSPSCQ *)spsc_queue, (CSPSCQ_DRAIN_FUNC)cspscq_consumer_do);
        if(0 < consumed)
        {
            fprintf(stdout, "[CONSUMER] ------------------ consumed %ld => marker %ld\n", consumed, g_spscq_marker);
        }
    }

    return (NULL);
}

int main(int argc, char **argv)
{
    CSPSCQ          spsc_queue;

    pthread_attr_t  attribute;
    pthread_t       producer_tid;
    pthread_t       consumer_tid;

    pthread_attr_init(&attribute);
    pthread_attr_setinheritsched(&attribute, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attribute, SCHED_OTHER);

    pthread_attr_setscope(&attribute, PTHREAD_SCOPE_PROCESS);
    pthread_attr_setstacksize(&attribute, 64 * 1024);
    pthread_attr_setguardsize(&attribute, 4 * 1024);

    cspscq_init(&spsc_queue, SPSCQ_QUEUE_CAPACITY);

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);
    assert(0 == pthread_create(&consumer_tid, &attribute, cspscq_consumer_thread, (void *)&spsc_queue));

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
    assert(0 == pthread_create(&producer_tid, &attribute, cspscq_producer_thread, (void *)&spsc_queue));

    pthread_join(producer_tid, NULL);
    //pthread_join(consumer_tid, NULL);

    cspscq_clean(&spsc_queue);

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

