/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_START_OPTIONS_IMPL_H
#define MOCK_START_OPTIONS_IMPL_H

#include "mock_types.h"

struct AbilityRuntime_StartOptions {
    OHOS::AAFwk::StartOptions GetInnerStartOptions() { return OHOS::AAFwk::StartOptions(); }
};

#endif // MOCK_START_OPTIONS_IMPL_H
