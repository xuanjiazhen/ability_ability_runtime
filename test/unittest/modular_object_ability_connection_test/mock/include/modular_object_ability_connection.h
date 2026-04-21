/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_MODULAR_OBJECT_ABILITY_CONNECTION_H
#define MOCK_MODULAR_OBJECT_ABILITY_CONNECTION_H

#include <mutex>
#include "ability_connection.h"

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectAbilityConnection : public AbilityConnection {
public:
    ModularObjectAbilityConnection() = default;
    ~ModularObjectAbilityConnection() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    std::mutex modularMutex_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_ABILITY_CONNECTION_H
