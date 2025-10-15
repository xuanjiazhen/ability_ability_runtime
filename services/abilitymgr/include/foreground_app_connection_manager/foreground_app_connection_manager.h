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

#ifndef OHOS_ABILITY_RUNTIME_FOREGROUND_APP_CONNECTION_MANAGER
#define OHOS_ABILITY_RUNTIME_FOREGROUND_APP_CONNECTION_MANAGER

#include <map>
#include <mutex>

#include "ability_record.h"
#include "foreground_app_connection_stub.h"
#include "nocopyable.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
struct ForegroundAppConnectionInfo {
    int32_t callerPid = -1;
    int32_t targetPid = -1;
    sptr<ForegroundAppConnection> observer = nullptr;
    std::vector<int32_t> abilityRecordIds;
};

class ForegroundAppConnectionManager {
public:
    static ForegroundAppConnectionManager& GetInstance();
    int32_t RegisterObserver(sptr<IForegroundAppConnection> observer);
    void UnregisterObserver(sptr<IForegroundAppConnection> observer);
    void OnConnected(ForegroundAppConnectionData &data);
    void OnDisconnected(ForegroundAppConnectionData &data);
    void OnCallerStarted(ForegroundAppConnectionData &data);

private:
    ForegroundAppConnectionManager() = default;
    ~ForegroundAppConnectionManager() = default;

    std::mutex oberverMutex_;
    std::vector<IForegroundAppConnection> observerList_;
    DISALLOW_COPY_AND_MOVE(ForegroundAppConnectionManager);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FOREGROUND_APP_CONNECTION_MANAGER