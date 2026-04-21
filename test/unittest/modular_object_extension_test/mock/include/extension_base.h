/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_EXTENSION_BASE_H
#define MOCK_EXTENSION_BASE_H

#include "extension.h"

namespace OHOS {
namespace AbilityRuntime {

template<class C>
class ExtensionBase : public Extension {
public:
    ExtensionBase() = default;
    virtual ~ExtensionBase() = default;

    void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override
    {
        Extension::Init(record, application, handler, token);
    }

    virtual std::shared_ptr<C> CreateAndInitContext(const std::shared_ptr<AbilityLocalRecord> &,
        const std::shared_ptr<OHOSApplication> &,
        std::shared_ptr<AbilityHandler> &,
        const sptr<IRemoteObject> &) { return nullptr; }

    std::shared_ptr<C> GetContext() { return context_; }
    std::shared_ptr<C> context_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_EXTENSION_BASE_H
