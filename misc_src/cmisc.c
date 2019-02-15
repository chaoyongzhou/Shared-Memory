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

#include "cmisc.h"

void *c_calloc(size_t size)
{
    void *ptr;

    ptr = malloc(size);

    if(NULL == ptr)
    {
        errno = ENOMEM;
        return (NULL);
    }

    memset(ptr, 0, size);

    return (ptr);
}

void c_free(void *ptr)
{
    free(ptr);
    return;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
