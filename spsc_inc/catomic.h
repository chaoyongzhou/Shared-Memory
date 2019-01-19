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

#ifndef _CATOMIC_H
#define _CATOMIC_H

#include <stdint.h>
#include <stdbool.h>

#define C_GET_VOLATILE(dst,src)                    \
do                                                 \
{                                                  \
    dst = src;                                     \
    __asm__ volatile("" ::: "memory");             \
} while(0)

#define C_PUT_ORDERED(dst,src)                     \
do                                                 \
{                                                  \
    __asm__ volatile("" ::: "memory");             \
    dst = src;                                     \
} while(0)

#define C_PUT_VOLATILE(dst,src)                    \
do                                                 \
{                                                  \
    __asm__ volatile("" ::: "memory");             \
    dst = src;                                     \
    __asm__ volatile("" ::: "memory");             \
} while(0)

#define C_GET_AND_ADD_INT64(original,dst,value)    \
do                                                 \
{                                                  \
    __asm__ volatile(                              \
        "lock; xaddq %0, %1"                       \
        : "=r"(original), "+m"(dst)                \
        : "0"(value));                             \
} while(0)

#define C_GET_AND_ADD_INT32(original,dst,value)    \
do                                                 \
{                                                  \
    __asm__ volatile(                              \
        "lock; xaddl %0, %1"                       \
        : "=r"(original), "+m"(dst)                \
        : "0"(value));                             \
} while(0)

#define C_CMPXCHG32(original,dst,expected,desired) \
do                                                 \
{                                                  \
    asm volatile(                                  \
        "lock; cmpxchgl %2, %1"                    \
        : "=a"(original), "+m"(dst)                \
        : "q"(desired), "0"(expected));            \
} while(0)

#define C_CMPXCHG64(original,dst,expected,desired) \
do                                                 \
{                                                  \
    asm volatile(                                  \
        "lock; cmpxchgq %2, %1"                    \
        : "=a"(original), "+m"(dst)                \
        : "q"(desired), "0"(expected));            \
} while(0)

inline bool c_cmpxchg64(volatile int64_t* destination,  int64_t expected, int64_t desired)
{
    int64_t original;

    __asm__ volatile(
        "lock; cmpxchgq %2, %1"
        : "=a"(original), "+m"(*destination)
        : "q"(desired), "0"(expected));

    return (original == expected);
}

inline bool c_cmpxchgu64(volatile uint64_t* destination,  uint64_t expected, uint64_t desired)
{
    uint64_t original;

    __asm__ volatile(
    "lock; cmpxchgq %2, %1"
    : "=a"(original), "+m"(*destination)
    : "q"(desired), "0"(expected));

    return (original == expected);
}

inline bool c_cmpxchg32(volatile int32_t* destination,  int32_t expected, int32_t desired)
{
    int32_t original;

    __asm__ volatile(
    "lock; cmpxchgl %2, %1"
    : "=a"(original), "+m"(*destination)
    : "q"(desired), "0"(expected));

    return (original == expected);
}

/* loadFence */
inline void c_acquire()
{
    volatile int64_t* dummy;

    __asm__ volatile("movq 0(%%rsp), %0" : "=r" (dummy) : : "memory");
}

/* storeFence */
inline void c_release()
{
    volatile int64_t dummy = 0;
    dummy;
}


#endif /*_CATOMIC_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
