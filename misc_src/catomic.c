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

#include "catomic.h"

bool c_cmpxchg64(volatile int64_t* destination,  int64_t expected, int64_t desired)
{
    int64_t original;

    __asm__ volatile(
        "lock; cmpxchgq %2, %1"
        : "=a"(original), "+m"(*destination)
        : "q"(desired), "0"(expected));

    return (original == expected);
}

bool c_cmpxchgu64(volatile uint64_t* destination,  uint64_t expected, uint64_t desired)
{
    uint64_t original;

    __asm__ volatile(
    "lock; cmpxchgq %2, %1"
    : "=a"(original), "+m"(*destination)
    : "q"(desired), "0"(expected));

    return (original == expected);
}

bool c_cmpxchg32(volatile int32_t* destination,  int32_t expected, int32_t desired)
{
    int32_t original;

    __asm__ volatile(
    "lock; cmpxchgl %2, %1"
    : "=a"(original), "+m"(*destination)
    : "q"(desired), "0"(expected));

    return (original == expected);
}

/* loadFence */
void c_acquire()
{
    volatile int64_t* dummy;

    __asm__ volatile("movq 0(%%rsp), %0" : "=r" (dummy) : : "memory");
}

/* storeFence */
void c_release()
{
    volatile int64_t dummy = 0;
    dummy;
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/
