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

#ifndef MOCK_ABILITY_MANAGER_CLIENT_H
#define MOCK_ABILITY_MANAGER_CLIENT_H

#include "oh_mock_utils.h"
#include "iability_manager_collaborator.h"
#include <memory>
#include <string>

namespace OHOS {
namespace AAFwk {

// Mock IAbilityManagerCollaborator - 仅实现 uripermmgr 需要的方法
class MockIAbilityManagerCollaborator : public IAbilityManagerCollaborator {
public:
    MockIAbilityManagerCollaborator() = default;
    ~MockIAbilityManagerCollaborator() override = default;

    // 实现 URI 权限相关的方法
    int32_t GrantUriPermission(const std::vector<std::string> &uriVec, uint32_t flag,
        uint32_t targetTokenId, const std::string &targetBundleName) override
    {
        if (grantUriPermissionFunc_) {
            return grantUriPermissionFunc_(uriVec, flag, targetTokenId, targetBundleName);
        }
        return 0;
    }

    int32_t RevokeUriPermission(uint32_t tokenId) override
    {
        if (revokeUriPermissionFunc_) {
            return revokeUriPermissionFunc_(tokenId);
        }
        return 0;
    }

    // 设置回调函数
    void SetGrantUriPermissionCallback(std::function<int32_t(
        const std::vector<std::string>&, uint32_t, uint32_t, const std::string&)> callback)
    {
        grantUriPermissionFunc_ = callback;
    }

    void SetRevokeUriPermissionCallback(std::function<int32_t(uint32_t)> callback)
    {
        revokeUriPermissionFunc_ = callback;
    }

    // 其他 IAbilityManagerCollaborator 方法返回默认值
    int32_t NotifyStartAbility(const AppExecFwk::AbilityInfo&, int32_t, Want&, uint64_t, int32_t) override { return 0; }
    int32_t NotifyPreloadAbility(const std::string&) override { return 0; }
    int32_t NotifyAbilityForeground(const std::string&, int32_t) override { return 0; }
    int32_t NotifyAbilityBackground(const std::string&, int32_t) override { return 0; }
    int32_t NotifyAbilityTransitionDone(const std::string&, int32_t) override { return 0; }
    int32_t NotifyProcessCollaboratorExit(const std::string&, int32_t) override { return 0; }
    int32_t NotifyAbilitySanitizerBlock(const std::string&, int32_t) override { return 0; }
    int32_t NotifyAbilitySanitizerUnblock(const std::string&, int32_t) override { return 0; }
    int32_t NotifyChangeAbilityVisibility(const std::string&, int32_t, bool) override { return 0; }
    int32_t NotifyProcessStatusChanged(const std::string&, int32_t, bool) override { return 0; }
    int32_t NotifyAppMemoryLevelChanged(int32_t) override { return 0; }
    int32_t NotifyChangeMemoryLevel(int32_t) override { return 0; }
    void NotifyChangeConfig(uint32_t) override {}
    int32_t NotifyRecoveryAbility(int32_t, int32_t) override { return 0; }
    void NotifyForceRecoveryAbility(int32_t, int32_t) {}
    int32_t NotifyStartSpecifiedAbilityFlag(const std::string&, int32_t) override { return 0; }
    int32_t NotifyTerminateAbility(const std::string&, int32_t, int32_t) override { return 0; }
    int32_t NotifyPrepareTerminateAbility(const std::string&, int32_t) override { return 0; }
    int32_t NotifyChangeUserVerification(int32_t, bool) override { return 0; }
    int32_t NotifyMissionOperatorCreated(int32_t, const sptr<IRemoteObject>&) override { return 0; }
    int32_t NotifyMissionOperatorDestroyed(int32_t) override { return 0; }
    int32_t NotifyMissionListenerCreated(const sptr<IRemoteObject>&) override { return 0; }
    int32_t NotifyMissionListenerDestroyed(const sptr<IRemoteObject>&) override { return 0; }
    int32_t NotifyWindowOperatorChanged(const sptr<IRemoteObject>&, int32_t, int32_t) override { return 0; }
    int32_t NotifyTransferMissionStateChanged(int32_t, int32_t) override { return 0; }
    int32_t NotifyUpdateSessionInfo(int32_t, const SessionInfo&) override { return 0; }
    int32_t NotifyGrantUriPermissionStart(const std::vector<std::string>&, uint32_t, int32_t) override { return 0; }
    int32_t NotifyGrantUriPermissionEnd(const std::vector<std::string>&, uint32_t, int32_t, int32_t) override { return 0; }
    int32_t NotifyRevokeUriPermission(const std::vector<std::string>&, uint32_t, int32_t) override { return 0; }

private:
    std::function<int32_t(const std::vector<std::string>&, uint32_t, uint32_t, const std::string&)> grantUriPermissionFunc_;
    std::function<int32_t(uint32_t)> revokeUriPermissionFunc_;
};

// Mock AbilityManagerClient - 单例模式
class MockAbilityManagerClient {
public:
    static std::shared_ptr<MockAbilityManagerClient> GetInstance()
    {
        static std::shared_ptr<MockAbilityManagerClient> instance = std::make_shared<MockAbilityManagerClient>();
        return instance;
    }

    void SetCollaborator(sptr<IAbilityManagerCollaborator> collaborator)
    {
        collaborator_ = collaborator;
    }

    sptr<IAbilityManagerCollaborator> GetAbilityManagerCollaborator()
    {
        return collaborator_;
    }

private:
    MockAbilityManagerClient() = default;
    ~MockAbilityManagerClient() = default;

    sptr<IAbilityManagerCollaborator> collaborator_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_ABILITY_MANAGER_CLIENT_H
