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
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>

#include <stdint.h>
#include <pthread.h>

#include "cmpscq.h"


#define MPSCQ_TEST_CAPACITY             (256)

#define MPSCQ_TEST_MAX_NUMBER           (100 * 10000)

#define MPSCQ_TEST_BITMAP_MAX_SIZE      ((MPSCQ_TEST_MAX_NUMBER + 7) / 8)

static uint8_t  g_mpscq_consumer_bitmap[ MPSCQ_TEST_BITMAP_MAX_SIZE ];

static void __cmpscq_consumer_bitmap_init()
{
    uint32_t    idx;

    for(idx = 0; idx < MPSCQ_TEST_BITMAP_MAX_SIZE; idx ++)
    {
        g_mpscq_consumer_bitmap[ idx ] = 0;
    }

    return;
}

static void __cmpscq_consumer_bitmap_set(const uint32_t bit_pos)
{
    uint32_t   byte_nth;
    uint32_t   bit_nth;
    uint8_t    e;

    byte_nth = (bit_pos >> 3); /*bit_pos / 8*/
    bit_nth  = (bit_pos & 7);  /*bit_pos % 8*/
    e        = (1 << bit_nth);

    assert(byte_nth < MPSCQ_TEST_BITMAP_MAX_SIZE);
    assert(0 == (g_mpscq_consumer_bitmap[ byte_nth ] & e));

    g_mpscq_consumer_bitmap[ byte_nth ] |= e;
    return;
}

static bool __cmpscq_consumer_bitmap_check(const uint32_t bit_pos)
{
    uint32_t   byte_nth;
    uint32_t   bit_nth;
    uint8_t    e;

    byte_nth = (bit_pos >> 3); /*bit_pos / 8*/
    bit_nth  = (bit_pos & 7);  /*bit_pos % 8*/
    e        = (1 << bit_nth);

    assert(byte_nth < MPSCQ_TEST_BITMAP_MAX_SIZE);

    return (0 < (g_mpscq_consumer_bitmap[ byte_nth ] & e));
}

void *cmpscq_producer_thread_1(void *spsc_queue)
{
    uint64_t        idx;

    for(idx = 1; idx < MPSCQ_TEST_MAX_NUMBER;)
    {
        if(EC_OFFER_SUCC == cmpscq_offer((volatile CMPSCQ *)spsc_queue, (void *)idx))
        {
            //fprintf(stdout, "[PRODUCER 1] offer %ld\n", idx);
            //fflush(stdout);

            idx += 2;
        }
    }
    return (NULL);
}

void *cmpscq_producer_thread_2(void *spsc_queue)
{
    uint64_t        idx;

    for(idx = 2; idx < MPSCQ_TEST_MAX_NUMBER;)
    {
        if(EC_OFFER_SUCC == cmpscq_offer((volatile CMPSCQ *)spsc_queue, (void *)idx))
        {
            //fprintf(stdout, "[PRODUCER 2] offer %ld\n", idx);
            //fflush(stdout);

            idx += 2;
        }
    }
    return (NULL);
}

void cmpscq_consumer_do(volatile void *item)
{
    uint64_t    idx;

    idx = (uint64_t)item;

    __cmpscq_consumer_bitmap_set((uint32_t)idx);

    return;
}

void *cmpscq_consumer_thread(void *spsc_queue)
{
    uint32_t    upper_marker  = 0;
    uint32_t    bottom_marker = 1;

    __cmpscq_consumer_bitmap_init();

    for(;;)
    {
        uint64_t    consumed;

        consumed = cmpscq_drain_all((volatile CMPSCQ *)spsc_queue, (CMPSCQ_DRAIN_FUNC)cmpscq_consumer_do);
        if(0 < consumed)
        {
            upper_marker += consumed;
            while(__cmpscq_consumer_bitmap_check(bottom_marker))
            {
                bottom_marker ++;
            }
            fprintf(stdout, "[CONSUMER] ------------------ consumed %ld => %u / %u\n",
                            consumed, bottom_marker, upper_marker);
        }
    }

    return (NULL);
}

int main(int argc, char **argv)
{
    CMPSCQ          spsc_queue;

    pthread_attr_t  attribute;
    pthread_t       producer_tid_1;
    pthread_t       producer_tid_2;
    pthread_t       consumer_tid;

    pthread_attr_init(&attribute);
    pthread_attr_setinheritsched(&attribute, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attribute, SCHED_OTHER);

    pthread_attr_setscope(&attribute, PTHREAD_SCOPE_PROCESS);
    pthread_attr_setstacksize(&attribute, 64 * 1024);
    pthread_attr_setguardsize(&attribute,  4 * 1024);

    cmpscq_init(&spsc_queue, MPSCQ_TEST_CAPACITY);

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);
    assert(0 == pthread_create(&consumer_tid, &attribute, cmpscq_consumer_thread, (void *)&spsc_queue));

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
    assert(0 == pthread_create(&producer_tid_1, &attribute, cmpscq_producer_thread_1, (void *)&spsc_queue));
    assert(0 == pthread_create(&producer_tid_2, &attribute, cmpscq_producer_thread_2, (void *)&spsc_queue));

    pthread_join(producer_tid_1, NULL);
    pthread_join(producer_tid_2, NULL);
    //pthread_join(consumer_tid, NULL);

    cmpscq_clean(&spsc_queue);

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

