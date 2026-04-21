/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
#define MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H

#include "native_extension/context_impl.h"
#include "errors.h"
#include "mock_types.h"

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectExtensionContext : public Context {
public:
    ModularObjectExtensionContext() = default;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
