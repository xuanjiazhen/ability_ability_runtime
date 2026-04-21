/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_NATIVE_CONTEXT_IMPL_H
#define MOCK_NATIVE_CONTEXT_IMPL_H

#include <memory>
#include "extension_ability_info.h"

namespace OHOS {
namespace AbilityRuntime {
class Context {};
} // namespace AbilityRuntime
} // namespace OHOS

struct AbilityRuntime_Context {
    OHOS::AppExecFwk::ExtensionAbilityType type;
    std::weak_ptr<OHOS::AbilityRuntime::Context> context;
};

#endif // MOCK_NATIVE_CONTEXT_IMPL_H
