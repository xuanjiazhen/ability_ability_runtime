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

#include <cstring>
#include <unistd.h>
#include <iostream>
#include <string>
#include <unordered_map>

#include "app_mgr_client.h"
#include "ability_manager_client.h"
#include "extension_manager_client.h"
#include "singleton.h"
#include "mock_native_token.h"
#include "ability_connect_callback.h"
#include "want.h"

using namespace std;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

int main(int argc, char* argv[])
{
    MockNativeToken::SetNativeToken();
    AAFwk::Want want;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    want.SetElementName("com.example.aaa", "MainAbility");
    AAFwk::AbilityManagerClient.GetInstance()->RequestModalUIExtension(want);
    AAFwk::ExtensionManagerClient.GetInstance()->ConnectExtensionAbility(want, callback);

    cout << "memory size changed success " <<  endl;
    while(1) {}
    return 0;
}