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

#ifndef _TYPE_H
#define _TYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef uint64_t            EC_BOOL;

#define EC_TRUE             ((EC_BOOL)  0)
#define EC_FALSE            ((EC_BOOL)  1)

#define EC_OFFER_SUCC       ((EC_BOOL)  0)
#define EC_OFFER_FULL       ((EC_BOOL) -1)
#define EC_OFFER_ERR        ((EC_BOOL) -2)

#endif /*_TYPE_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

