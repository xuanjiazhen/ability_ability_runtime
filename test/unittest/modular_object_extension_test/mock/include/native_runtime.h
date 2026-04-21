/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_NATIVE_RUNTIME_H
#define MOCK_NATIVE_RUNTIME_H

#include <string>
#include "native_extension/extension_ability_impl.h"

namespace OHOS {
namespace AbilityRuntime {

class NativeRuntime {
public:
    static bool g_loadModuleResult;
    static bool LoadModule(const std::string &bundleModuleName, const std::string &fileName,
        const std::string &abilityName, AbilityRuntime_ExtensionInstance &instance)
    {
        return g_loadModuleResult;
    }
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_NATIVE_RUNTIME_H
