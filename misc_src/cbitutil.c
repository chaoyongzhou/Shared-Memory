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

#include "cbitutil.h"

/* Taken from Hacker's Delight as ntz10 at http://www.hackersdelight.org/hdcodetxt/ntz.c.txt */
int c_number_of_trailing_zeroes(int32_t value)
{
    return __builtin_ctz(value);
}

int c_number_of_leading_zeroes(int32_t value)
{
    return __builtin_clz(value);
}

int32_t c_find_next_power_of_two(int32_t value)
{
    size_t    i;

    value--;

    /*
     * Set all bits below the leading one using binary expansion
     * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
     */
    for (i = 1; i < sizeof(value) * 8; i = i * 2)
    {
        value |= (value >> i);
    }

    return value + 1;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
