/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "extension_config_mgr.h"

#include <cctype>
#include <fstream>

#include "app_module_checker.h"
#include "cJSON.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS::AbilityRuntime {
namespace {
    constexpr char EXTENSION_BLOCKLIST_FILE_PATH[] = "/system/etc/extension_blocklist_config.json";
}

void ExtensionConfigMgr::Init()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Init begin");
    blocklistConfig_.clear();
    extensionBlocklist_.clear();
    extensionEtsBlocklist_.clear();

    // read blocklist from extension_blocklist_config.json
    std::ifstream inFile;
    inFile.open(EXTENSION_BLOCKLIST_FILE_PATH, std::ios::in);
    if (!inFile.is_open()) {
        TAG_LOGE(AAFwkTag::EXT, "read extension config error");
        return;
    }
    std::string fileContent((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    cJSON *extensionConfig = cJSON_Parse(fileContent.c_str());
    if (extensionConfig == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extension config json parse error");
        return;
    }
    cJSON *blockListItem = cJSON_GetObjectItem(extensionConfig, ExtensionConfigItem::ITEM_NAME_BLOCKLIST);
    if (blockListItem == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extension config file have no blocklist node");
        cJSON_Delete(extensionConfig);
        return;
    }
    std::unordered_set<std::string> currentBlockList;
    cJSON *childItem = blockListItem->child;
    while (childItem != nullptr) {
        if (!cJSON_IsArray(childItem)) {
            continue;
        }
        int size = cJSON_GetArraySize(childItem);
        for (int i = 0; i < size; i++) {
            cJSON *item = cJSON_GetArrayItem(childItem, i);
            if (item != nullptr && cJSON_IsString(item)) {
                std::string value = item->valuestring;
                currentBlockList.emplace(value);
            }
        }
        std::string key = childItem->string == nullptr ? "" : childItem->string;
        blocklistConfig_.emplace(key, std::move(currentBlockList));
        currentBlockList.clear();

        childItem = childItem->next;
    }
    cJSON_Delete(extensionConfig);
    TAG_LOGD(AAFwkTag::EXT, "Init end");
}

void ExtensionConfigMgr::AddBlockListItem(const std::string& name, int32_t type)
{
    TAG_LOGD(AAFwkTag::EXT, "name: %{public}s, type: %{public}d", name.c_str(), type);
    auto iter = blocklistConfig_.find(name);
    if (iter == blocklistConfig_.end()) {
        TAG_LOGD(AAFwkTag::EXT, "Extension name: %{public}s not exist", name.c_str());
        return;
    }
    extensionBlocklist_.emplace(type, iter->second);
}

void ExtensionConfigMgr::UpdateRuntimeModuleChecker(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    if (!runtime) {
        TAG_LOGE(AAFwkTag::EXT, "null runtime");
        return;
    }
    TAG_LOGD(AAFwkTag::EXT, "extensionType_: %{public}d", extensionType_);
    GenerateExtensionEtsBlocklists();
    if (runtime->GetLanguage() == AbilityRuntime::Runtime::Language::ETS) {
        TAG_LOGD(AAFwkTag::EXT, "ets runtime");
        SetExtensionEtsCheckCallback(runtime);
    } else {
        TAG_LOGD(AAFwkTag::EXT, "not ets runtime");
        auto moduleChecker = std::make_shared<AppModuleChecker>(extensionType_, extensionBlocklist_);
        runtime->SetModuleLoadChecker(moduleChecker);
        extensionBlocklist_.clear();
    }
}

void ExtensionConfigMgr::GenerateExtensionEtsBlocklists()
{
    if (!extensionEtsBlocklist_.empty()) {
        TAG_LOGD(AAFwkTag::EXT, "extension ets block list not empty.");
        return;
    }
    auto iter = extensionBlocklist_.find(extensionType_);
    if (iter == extensionBlocklist_.end()) {
        TAG_LOGD(AAFwkTag::EXT, "null extension block, extensionType: %{public}d.", extensionType_);
        return;
    }
    for (const auto& module: iter->second) {
        std::string lowermodule;
        lowermodule.reserve(module.size());
        std::transform(module.cbegin(), module.cend(), std::back_inserter(lowermodule),
            [](unsigned char c) { return std::tolower(c); });
        extensionEtsBlocklist_.emplace(std::move(lowermodule));
    }
}

std::string ExtensionConfigMgr::GetStringAfterRemovePreFix(const std::string &name)
{
    if (name.length() >= ExtensionConfigItem::ITEM_LENGTH_OHOSPREFIX &&
        (name.compare(0, ExtensionConfigItem::ITEM_LENGTH_OHOSPREFIX, ExtensionConfigItem::ITEM_NAME_OHOSPREFIX)
        == 0)) {
        return name.substr(ExtensionConfigItem::ITEM_LENGTH_OHOSPREFIX);
    } else if (name.length() >= ExtensionConfigItem::ITEM_LENGTH_HMSPREFIX &&
        (name.compare(0, ExtensionConfigItem::ITEM_LENGTH_HMSPREFIX, ExtensionConfigItem::ITEM_NAME_HMSPREFIX)
        == 0)) {
        return name.substr(ExtensionConfigItem::ITEM_LENGTH_HMSPREFIX);
    }
    return name;
}

bool ExtensionConfigMgr::CheckEtsModuleLoadable(const std::string &className, const std::string &fileName)
{
    std::string lowerClassName;
    std::string lowerFileName;
    lowerClassName.reserve(className.size());
    lowerFileName.reserve(fileName.size());
    std::transform(className.cbegin(), className.cend(),
        std::back_inserter(lowerClassName), [](unsigned char c) { return std::tolower(c); });
    std::transform(fileName.cbegin(), fileName.cend(),
        std::back_inserter(lowerFileName), [](unsigned char c) { return std::tolower(c); });
    TAG_LOGD(AAFwkTag::EXT, "extensionType: %{public}d, className is %{public}s, fileName is %{public}s",
        extensionType_, className.c_str(), fileName.c_str());
    if (lowerFileName.size() >= lowerClassName.size() ||
        lowerClassName.compare(0, lowerFileName.size(), lowerFileName) != 0 ||
        ((lowerClassName.compare(0, lowerFileName.size(), lowerFileName) == 0) &&
          lowerClassName[lowerFileName.size()] != '.')) {
        TAG_LOGE(AAFwkTag::EXT, "The %{public}s is not the name of the file where the %{public}s is located.",
            fileName.c_str(), className.c_str());
        return false;
    }

    std::string fileNameAfterRemovePreFix = GetStringAfterRemovePreFix(lowerFileName);
    for (const auto& blockClassName: extensionEtsBlocklist_) {
        if (blockClassName == fileNameAfterRemovePreFix) {
            TAG_LOGE(AAFwkTag::EXT, "moduleName is %{public}s full match in blocklist.", fileName.c_str());
            return false;
        }
    }
    TAG_LOGD(AAFwkTag::EXT, "className is %{public}s in not blocklist.", className.c_str());
    return true;
}

void ExtensionConfigMgr::SetExtensionEtsCheckCallback(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    auto callback = [extensionConfigMgrWeak = weak_from_this()](const std::string &className,
        const std::string &fileName) -> bool {
        auto extensionConfigMgr = extensionConfigMgrWeak.lock();
        if (extensionConfigMgr == nullptr) {
            TAG_LOGD(AAFwkTag::EXT, "null extensionConfigMgr");
            return false;
        }
        return extensionConfigMgr->CheckEtsModuleLoadable(className, fileName);
    };
    runtime->SetExtensionApiCheckCallback(callback);
}
}