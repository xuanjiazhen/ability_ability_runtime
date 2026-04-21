/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_WANT_UTILS_H
#define MOCK_WANT_UTILS_H

#include "ability_runtime_common.h"

struct AbilityBase_Want;

extern AbilityRuntime_ErrorCode g_checkWantResult;

AbilityRuntime_ErrorCode CheckWant(AbilityBase_Want *want);

#endif // MOCK_WANT_UTILS_H
