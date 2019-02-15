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

#include "cmpscrb.h"


#define MPSCRB_TEST_CAPACITY                (1 << 20) /*1MB*/

#define MPSCRB_TEST_MAX_NUMBER              (1000 * 10000)

#define MPSCRB_TEST_BITMAP_MAX_SIZE         ((MPSCRB_TEST_MAX_NUMBER + 7) / 8)

static uint8_t  g_mpscrb_consumer_bitmap[ MPSCRB_TEST_BITMAP_MAX_SIZE ];

static void __cmpscrb_consumer_bitmap_init()
{
    uint32_t    idx;

    for(idx = 0; idx < MPSCRB_TEST_BITMAP_MAX_SIZE; idx ++)
    {
        g_mpscrb_consumer_bitmap[ idx ] = 0;
    }

    return;
}

static void __cmpscrb_consumer_bitmap_set(const uint32_t bit_pos)
{
    uint32_t   byte_nth;
    uint32_t   bit_nth;
    uint8_t    e;

    byte_nth = (bit_pos >> 3); /*bit_pos / 8*/
    bit_nth  = (bit_pos & 7);  /*bit_pos % 8*/
    e        = (1 << bit_nth);

    assert(byte_nth < MPSCRB_TEST_BITMAP_MAX_SIZE);
    assert(0 == (g_mpscrb_consumer_bitmap[ byte_nth ] & e));

    g_mpscrb_consumer_bitmap[ byte_nth ] |= e;
    return;
}

static bool __cmpscrb_consumer_bitmap_check(const uint32_t bit_pos)
{
    uint32_t   byte_nth;
    uint32_t   bit_nth;
    uint8_t    e;

    byte_nth = (bit_pos >> 3); /*bit_pos / 8*/
    bit_nth  = (bit_pos & 7);  /*bit_pos % 8*/
    e        = (1 << bit_nth);

    assert(byte_nth < MPSCRB_TEST_BITMAP_MAX_SIZE);

    return (0 < (g_mpscrb_consumer_bitmap[ byte_nth ] & e));
}

static size_t __cmpscrb_set_msg(char *msg, size_t size, uint64_t num)
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

static uint64_t __cmpscrb_check_msg(char *msg, size_t size)
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
        if(msg[ pos ] != msg[ pos % (len + 1) ])
        {
            fprintf(stdout, "[CHECK] error: size %ld, pos %ld, len %ld, num %ld, %c != %c\n",
                            size, pos, len, num,
                            msg[ pos ], msg[ pos % (len + 1) ]);

            fprintf(stdout, "[CHECK] %.*s => %ld\n", (int)size, msg, num);
            fflush(stdout);
        }
        assert(msg[ pos ] == msg[ pos % (len + 1) ]);
    }

    //fprintf(stdout, "[CHECK] %.*s => %ld\n", 16, msg, num);
    //fflush(stdout);

    return (num);
}

void *cmpscrb_producer_thread_1(void *cmpscrb)
{
    uint64_t        idx;
    char            msg[1 * 1024];

    static __thread uint64_t    produced = 0;

    assert(CMPSCRB_MESSAGE_MAX_LENGTH(MPSCRB_TEST_CAPACITY) >= sizeof(msg)/sizeof(msg[0]));

    for(idx = 1; idx < MPSCRB_TEST_MAX_NUMBER;)
    {
        size_t      len;
        EC_BOOL     ret;

        len = __cmpscrb_set_msg(msg, sizeof(msg)/sizeof(msg[0]), idx);

        ret = cmpscrb_write((volatile CMPSCRB *)cmpscrb, (void *)msg, len);

        if(EC_OFFER_SUCC == ret)
        {
            produced += len;

            //fprintf(stdout, "[PRODUCER 1] offer %ld => produced %ld\n", idx, produced);
            //fflush(stdout);

            idx += 2;
            continue;
        }

        if(EC_OFFER_ERR == ret)
        {
            fprintf(stdout, "[PRODUCER 1] offer %ld => produced %ld => failed\n", idx, produced);
            fflush(stdout);
            break;
        }

        /*full => write again*/
    }

    fprintf(stdout, "[PRODUCER 1] complete\n");
    fflush(stdout);
    return (NULL);
}

void *cmpscrb_producer_thread_2(void *cmpscrb)
{
    uint64_t        idx;
    char            msg[1 * 1024];

    static __thread uint64_t    produced = 0;

    assert(CMPSCRB_MESSAGE_MAX_LENGTH(MPSCRB_TEST_CAPACITY) >= sizeof(msg)/sizeof(msg[0]));

    for(idx = 2; idx < MPSCRB_TEST_MAX_NUMBER;)
    {
        size_t      len;
        EC_BOOL     ret;

        len = __cmpscrb_set_msg(msg, sizeof(msg)/sizeof(msg[0]), idx);

        ret = cmpscrb_write((volatile CMPSCRB *)cmpscrb, (void *)msg, len);

        if(EC_OFFER_SUCC == ret)
        {
            produced += len;

            //fprintf(stdout, "[PRODUCER 2] offer %ld => produced %ld\n", idx, produced);
            //fflush(stdout);

            idx += 2;
            continue;
        }

        if(EC_OFFER_ERR == ret)
        {
            fprintf(stdout, "[PRODUCER 2] offer %ld => produced %ld => failed\n", idx, produced);
            fflush(stdout);
            break;
        }

        //fprintf(stdout, "[PRODUCER 2] offer %ld => full\n", idx);
        //fflush(stdout);

        /*full => write again*/
    }

    fprintf(stdout, "[PRODUCER 2] complete\n");
    fflush(stdout);

    return (NULL);
}

void cmpscrb_consumer_do(char *msg, size_t size)
{
    uint64_t    idx;

    idx = __cmpscrb_check_msg(msg, size);

    __cmpscrb_consumer_bitmap_set((uint32_t)idx);

    return;
}

void *cmpscrb_consumer_thread(void *cmpscrb)
{
    uint32_t    upper_marker  = 0;
    uint32_t    bottom_marker = 1;

    __cmpscrb_consumer_bitmap_init();

    for(;;)
    {
        uint64_t    consumed;

        consumed = cmpscrb_read((volatile CMPSCRB *)cmpscrb, (CMPSCRB_DRAIN_FUNC)cmpscrb_consumer_do, 128);
        if(0 < consumed)
        {
            upper_marker += consumed;
            while(MPSCRB_TEST_MAX_NUMBER > bottom_marker
            && __cmpscrb_consumer_bitmap_check(bottom_marker))
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
    CMPSCRB          cmpscrb;

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

    cmpscrb_init(&cmpscrb, MPSCRB_TEST_CAPACITY + CMPSCRB_TRAILER_LENGTH);

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
    assert(0 == pthread_create(&consumer_tid, &attribute, cmpscrb_consumer_thread, (void *)&cmpscrb));

    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
    assert(0 == pthread_create(&producer_tid_1, &attribute, cmpscrb_producer_thread_1, (void *)&cmpscrb));
    assert(0 == pthread_create(&producer_tid_2, &attribute, cmpscrb_producer_thread_2, (void *)&cmpscrb));

    //pthread_join(consumer_tid, NULL);
    pthread_join(producer_tid_1, NULL);
    pthread_join(producer_tid_2, NULL);

    cmpscrb_clean(&cmpscrb);

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

