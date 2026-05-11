/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_APP_MGR_CLIENT_H
#define OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_APP_MGR_CLIENT_H

#include <string>
#include <vector>

#include "app_mgr_constants.h"
#include "iapplication_state_observer.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrClient {
public:
    AppMgrClient();
    ~AppMgrClient();
    AppMgrResultCode ConnectAppMgrService();
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList);
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_APP_MGR_CLIENT_H
