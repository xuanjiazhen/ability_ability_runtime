/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
#define MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H

#include "extension_context.h"
#include <cstddef>

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectExtensionContext : public ExtensionContext {
public:
    static const size_t CONTEXT_TYPE_ID;

    ErrCode StartSelfUIAbility(const AAFwk::Want &want) const
    {
        return AAFwk::AbilityManagerClient::GetInstance()->StartSelfUIAbility(want);
    }

    ErrCode StartSelfUIAbilityWithStartOptions(const AAFwk::Want &want,
        const AAFwk::StartOptions &startOptions) const
    {
        return AAFwk::AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithStartOptions(want, startOptions);
    }

    ErrCode TerminateSelf()
    {
        return AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    }

    bool IsContext(size_t contextTypeId) { return contextTypeId == CONTEXT_TYPE_ID; }
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
