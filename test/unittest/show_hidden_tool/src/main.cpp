/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>

#include "app_mgr_client.h"
#include "application_state_observer_stub.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"
#include "singleton.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

volatile int32_t g_count = 0;

class MockApplicationStateObserver : public ApplicationStateObserverStub {
public:
    MockApplicationStateObserver() = default;
    virtual ~MockApplicationStateObserver() = default;

    void OnWindowShow(const ProcessData &processData) override
    {
        g_count++;
        cout << "OnWindowShow:" << endl;
        cout << "    pid: " << processData.pid << endl;
        cout << "    uid: " << processData.uid << endl;
        cout << "    hostPid: " << processData.hostPid << endl;
        cout << "    windowState: " << static_cast<int>(processData.windowState) << endl;
    }

    void OnWindowHidden(const ProcessData &processData) override
    {
        g_count++;
        cout << "OnWindowHidden:" << endl;
        cout << "    pid: " << processData.pid << endl;
        cout << "    uid: " << processData.uid << endl;
        cout << "    hostPid: " << processData.hostPid << endl;
        cout << "    windowState: " << static_cast<int>(processData.windowState) << endl;
    }

    void OnForegroundApplicationChanged(const AppStateData &appStateData) override
    {
        g_count++;
        cout << "OnForegroundApplicationChanged:" << endl;
        cout << "    pid: " << appStateData.pid << endl;
        cout << "    uid: " << appStateData.uid << endl;
        cout << "    state: " << appStateData.state << endl;
    }
};

int main()
{
    MockNativeToken::SetNativeToken();
    MyFlag::flag_ = ERR_OK;
    sptr<MockApplicationStateObserver> observer_ = new MockApplicationStateObserver();
    int32_t ret = DelayedSingleton<AppMgrClient>::GetInstance()->RegisterApplicationStateObserver(observer_);
    if (ret == ERR_OK) {
        cout << "Register success." << ret << endl;
    } else {
        cout << "Register filed. error:" << ret << endl;
        return 0;
    }

    while (g_count < 100) {}
    
    return 0;
}