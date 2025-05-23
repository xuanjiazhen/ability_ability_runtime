/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_OHOS_AAFWK_CONNECTION_STATE_MANAGER_H
#define MOCK_OHOS_AAFWK_CONNECTION_STATE_MANAGER_H

#include <vector>

#include "task_handler_wrap.h"
#include "mock_common.h"

namespace OHOS {
namespace AAFwk {
class ConnectionStateManager : public std::enable_shared_from_this<ConnectionStateManager> {
    DECLARE_DELAYED_SINGLETON(ConnectionStateManager)
public:

    void Init(const std::shared_ptr<TaskHandlerWrap> &handler = nullptr) {}

    int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
    {
        return 0;
    }

    int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
    {
        return 0;
    }

#ifdef WITH_DLP
    void GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos) {}
#endif // WITH_DLP

    void GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData) {}
};
}  // namespace AAFwk

namespace BackgroundTaskMgr {
class BackgroundTaskMgrHelper {
public:
    static ErrCode SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
    {
        return 0;
    }

    static ErrCode UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
    {
        return 0;
    }
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif
