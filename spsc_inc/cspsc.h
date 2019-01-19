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

#ifndef _CSPSC_H
#define _CSPSC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "cbitutil.h"


typedef uint64_t            CSPSC_BOOL;

#define CSPSC_OFFER_SUCC    ((CSPSC_BOOL)  0)
#define CSPSC_OFFER_ERR     ((CSPSC_BOOL) -2)
#define CSPSC_OFFER_FULL    ((CSPSC_BOOL) -1)

#define CSPSC_TRUE          ((CSPSC_BOOL)  0)
#define CSPSC_FALSE         ((CSPSC_BOOL)  1)


#endif/*_CSPSC_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/


