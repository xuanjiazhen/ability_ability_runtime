/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "agent_card_mgr.h"

#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "json_utils.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *KEEP_ALIVE_STORAGE_DIR = "/data/service/el1/public/database/agent_runtime";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_USER_ID = "userId";
const std::string JSON_KEY_URL = "url";
} // namespace

AgentCardMgr &AgentCardMgr::GetInstance()
{
    static AgentCardMgr instance;
    return instance;
}

AgentCardMgr::AgentCardMgr() {}

AgentCardMgr::~AgentCardMgr() {}

int32_t AgentCardMgr::HandleBundleInstall(const std::string &bundleName, int32_t userId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid bundleName");
        return -1;
    }
    BundleInfo bundleInfo;
    int32_t flag = BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO | BundleFlag::GET_BUNDLE_WITH_ABILITIES;
    bool result = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetBundleInfo(bundleName, flag,
        bundleInfo, userId);
    if (!result) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Get Bundle Info fail");
        return -1;
    }
    for (auto const &extensionInfo : bundleInfo.extensionInfos) {
        if (extensionInfo != ExtensionAbilityType::BACKUP) {
            continue;
        }
        std::vector<std::string> profileInfos{};
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetBundleInfo(extensionInfo, "agentCards",
            profileInfos);
    }
}

int32_t AgentCardMgr::HandleBundleUpdate(const std::string &bundleName, int32_t userId)
{

}

int32_t AgentCardMgr::HandleBundleRemove(const std::string &bundleName, int32_t userId)
{

}
} // namespace AgentRuntime
} // namespace OHOS
