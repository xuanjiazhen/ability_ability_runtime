/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_IAPPLICATION_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_IAPPLICATION_STATE_OBSERVER_H

#include <iremote_object.h>
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
class IApplicationStateObserver : public IRemoteBroker {
public:
    IApplicationStateObserver() = default;
    virtual ~IApplicationStateObserver() = default;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_IAPPLICATION_STATE_OBSERVER_H
