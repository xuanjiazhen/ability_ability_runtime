/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_NATIVE_EXTENSION_ABILITY_IMPL_H
#define MOCK_NATIVE_EXTENSION_ABILITY_IMPL_H

#include <memory>
#include "extension_ability_info.h"

namespace OHOS {
namespace AbilityRuntime {
class Extension;
} // namespace AbilityRuntime
} // namespace OHOS

struct AbilityRuntime_ExtensionInstance {
    OHOS::AppExecFwk::ExtensionAbilityType type;
    std::weak_ptr<OHOS::AbilityRuntime::Extension> extension;
};

typedef struct AbilityRuntime_ExtensionInstance *AbilityRuntime_ExtensionInstanceHandle;

#endif // MOCK_NATIVE_EXTENSION_ABILITY_IMPL_H
