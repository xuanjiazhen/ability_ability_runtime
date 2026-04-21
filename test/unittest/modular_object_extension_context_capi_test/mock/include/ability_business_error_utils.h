/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_ABILITY_BUSINESS_ERROR_UTILS_H
#define MOCK_ABILITY_BUSINESS_ERROR_UTILS_H

#include "ability_runtime_common.h"
#include <cstdint>

inline AbilityRuntime_ErrorCode ConvertToCommonBusinessErrorCode(int32_t err)
{
    if (err == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

inline AbilityRuntime_ErrorCode ConvertToAPI17BusinessErrorCode(int32_t err)
{
    if (err == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

#endif // MOCK_ABILITY_BUSINESS_ERROR_UTILS_H
