/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_SECUREC_H
#define MOCK_SECUREC_H

#include <cstring>
#include <cstddef>

#ifndef EOK
#define EOK 0
#endif

inline int strcpy_s(char *dest, size_t destMax, const char *src)
{
    if (dest == nullptr || src == nullptr || destMax == 0) {
        return -1;
    }
    size_t len = strlen(src);
    if (len >= destMax) {
        return -1;
    }
    memcpy(dest, src, len + 1);
    return EOK;
}

#endif // MOCK_SECUREC_H
