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
#ifndef MOCK_OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H
#define MOCK_OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H

#include <list>
#include <vector>
#include <string>
#include <unordered_set>

#include "ability_record.h"
#include "bundle_mgr_helper.h"
#include "system_dialog_scheduler.h"

namespace OHOS {
namespace EcologicalRuleMgrService {
struct AbilityCallerInfo;
}
namespace AAFwk {
struct IdentityNode {
    int32_t tokenId;
    std::string identity;
    IdentityNode(int tokenId, std::string identity) : tokenId(tokenId), identity(identity)
    {}
};

struct AddInfoParam {
    bool isExtension = false;
    bool isMoreHapList = false;
    bool withDefault = false;
    bool isExistDefaultApp = false;
    int32_t userId = 0;
    std::string typeName;
    std::vector<std::string> infoNames;
    AppExecFwk::AbilityInfo info;
};

struct GenerateRequestParam {
    bool isMoreHapList = false;
    bool findDefaultApp = false;
    bool isAppCloneSelector = false;
    bool fromImplicitStartAG = false;
};

using namespace OHOS::EcologicalRuleMgrService;
using ErmsCallerInfo = OHOS::EcologicalRuleMgrService::AbilityCallerInfo;

class ImplicitStartProcessor : public std::enable_shared_from_this<ImplicitStartProcessor> {
public:
    explicit ImplicitStartProcessor() = default;
    virtual ~ImplicitStartProcessor() = default;

    static bool IsImplicitStartAction(const Want &want);

    int ImplicitStartAbility(AbilityRequest &request, int32_t userId, int32_t windowMode = 0,
        const std::string &replaceWantString = "", bool isAppCloneSelector = false);

    void ResetCallingIdentityAsCaller(int32_t tokenId, bool flag);

    void RemoveIdentity(int32_t tokenId);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif