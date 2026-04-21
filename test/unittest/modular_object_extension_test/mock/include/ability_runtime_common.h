/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_ABILITY_RUNTIME_COMMON_H
#define MOCK_ABILITY_RUNTIME_COMMON_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ABILITY_RUNTIME_ERROR_CODE_NO_ERROR = 0,
    ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID = 401,
    ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE = 16000002,
    ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST = 16000011,
    ABILITY_RUNTIME_ERROR_CODE_INTERNAL = 16000050,
} AbilityRuntime_ErrorCode;

typedef struct AbilityRuntime_Context *AbilityRuntime_ContextHandle;

#ifdef __cplusplus
}
#endif

#endif // MOCK_ABILITY_RUNTIME_COMMON_H
