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

#ifndef _CBITUTIL_H
#define _CBITUTIL_H

#include <stdint.h>
#include <stddef.h>

#define C_CACHE_LINE_LENGTH (64)

#define C_ALIGN(value,alignment) (((value) + ((alignment) - 1)) & ~((alignment) - 1))

#define C_IS_POWER_OF_TWO(value) ((value) > 0 && (((value) & (~(value) + 1)) == (value)))

#define C_MIN(a,b) ((a) < (b) ? (a) : (b))

/* Taken from Hacker's Delight as ntz10 at http://www.hackersdelight.org/hdcodetxt/ntz.c.txt */
int c_number_of_trailing_zeroes(int32_t value);

int c_number_of_leading_zeroes(int32_t value);

int32_t c_find_next_power_of_two(int32_t value);

#endif /*_CBITUTIL_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
