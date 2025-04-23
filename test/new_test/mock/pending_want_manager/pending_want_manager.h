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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_PENDING_WANT_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_PENDING_WANT_MANAGER_H

#include <mutex>
#include <memory>
#include <map>
#include <vector>
#include <string>

#include "ability_record.h"
#include "pending_want_record.h"
#include "task_handler_wrap.h"
#include "want_sender_info.h"
#include "mock_common.h"

namespace OHOS {
namespace AAFwk {
class PendingWantManager : public std::enable_shared_from_this<PendingWantManager> {
public:
    PendingWantManager() {}
    explicit PendingWantManager(const std::shared_ptr<PendingWantManager> &manager) {}
    virtual ~PendingWantManager() {}

public:
    sptr<IWantSender> GetWantSender(int32_t callingUid, int32_t uid, const bool isSystemApp,
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, int32_t appIndex = 0)
    {
        return nullptr;
    }
    int32_t SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo)
    {
        return 0;
    }
    void CancelWantSender(const bool isSystemAppCall, const sptr<IWantSender> &sender) {}

    int32_t GetPendingWantUid(const sptr<IWantSender> &target)
    {
        return 0;
    }
    int32_t GetPendingWantUserId(const sptr<IWantSender> &target)
    {
        return 0;
    }
    std::string GetPendingWantBundleName(const sptr<IWantSender> &target)
    {
        return "";
    }
    int32_t GetPendingWantCode(const sptr<IWantSender> &target)
    {
        return 0;
    }
    int32_t GetPendingWantType(const sptr<IWantSender> &target)
    {
        return 0;
    }
    void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier) {}
    void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier) {}
    int32_t GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
    {
        return 0;
    }
    int32_t GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
    {
        return 0;
    }

    void Dump(std::vector<std::string> &info) {}
    void DumpByRecordId(std::vector<std::string> &info, const std::string &args) {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
