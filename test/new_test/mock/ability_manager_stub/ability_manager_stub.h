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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H

#include "ability_manager_interface.h"

#include <iremote_object.h>
#include <iremote_stub.h>
#include "mock_common.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerStub : public IRemoteStub<IAbilityManager> {
public:
    AbilityManagerStub() {}
    ~AbilityManagerStub() {}

    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) override
    {
        return 0;
    }

    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) override
    {
        return 0;
    }

    virtual int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
    {
        return 0;
    }

    virtual int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
    {
        return 0;
    }

#ifdef WITH_DLP
    virtual int GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos)
    {
        return 0;
    }
#endif // WITH_DLP

    virtual int GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData)
    {
        return 0;
    }

    virtual void CancelWantSenderByFlags(const sptr<IWantSender> &sender, uint32_t flags) {}

    virtual int Dump(int fd, const std::vector<std::u16string> &args)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
