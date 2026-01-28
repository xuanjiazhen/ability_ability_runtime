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

#include "ability_manager_client.h"

namespace OHOS {
namespace AAFwk {

// 静态成员变量初始化
std::shared_ptr<AbilityManagerClient> AbilityManagerClient::instance_ = nullptr;
sptr<IAbilityManagerCollaborator> AbilityManagerClient::mockCollaborator_ = nullptr;

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = std::make_shared<AbilityManagerClient>();
    }
    return instance_;
}

sptr<IAbilityManagerCollaborator> AbilityManagerClient::GetAbilityManagerCollaborator()
{
    return mockCollaborator_;
}

void AbilityManagerClient::SetMockCollaborator(sptr<IAbilityManagerCollaborator> collaborator)
{
    mockCollaborator_ = collaborator;
}

void AbilityManagerClient::ResetMock()
{
    mockCollaborator_ = nullptr;
    instance_ = nullptr;
}

} // namespace AAFwk
} // namespace OHOS
