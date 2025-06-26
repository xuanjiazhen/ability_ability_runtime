/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <fstream>
#include <nlohmann/json.hpp>

#include "app_module_checker.h"
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
    // clear cached data
    blocklistConfig_.clear();
    extensionBlocklist_.clear();

    // read blocklist from extension_blocklist_config.json
    std::ifstream inFile;
    inFile.open(EXTENSION_BLOCKLIST_FILE_PATH, std::ios::in);
    if (!inFile.is_open()) {
        TAG_LOGE(AAFwkTag::EXT, "read extension config error");
        return;
    }
    nlohmann::json extensionConfig;
    inFile >> extensionConfig;
    if (extensionConfig.is_discarded()) {
        TAG_LOGE(AAFwkTag::EXT, "extension config json discarded error");
        inFile.close();
        return;
    }
    if (!extensionConfig.contains(ExtensionConfigItem::ITEM_NAME_BLOCKLIST)) {
        TAG_LOGE(AAFwkTag::EXT, "extension config file have no blocklist node");
        inFile.close();
        return;
    }
    auto blackList = extensionConfig.at(ExtensionConfigItem::ITEM_NAME_BLOCKLIST);
    std::unordered_set<std::string> currentBlockList;
    for (const auto& item : blackList.items()) {
        if (!blackList[item.key()].is_array()) {
            continue;
        }
        for (const auto& value : blackList[item.key()]) {
            currentBlockList.emplace(value.get<std::string>());
        }
        blocklistConfig_.emplace(item.key(), std::move(currentBlockList));
        currentBlockList.clear();
    }
    if (!extensionConfig.contains(ExtensionConfigItem::ITEM_NAME_REPLACEMODULELIST)) {
        TAG_LOGE(AAFwkTag::EXT, "extension config file have no replaceList node");
        inFile.close();
        return;
    }
    auto replaceList = extensionConfig.at(ExtensionConfigItem::ITEM_NAME_REPLACEMODULELIST);
    for (const auto& item : replaceList.items()) {
        if (!replaceList[item.key()].is_string()) {
            continue;
        }
        replaceModuleListConfig_.emplace(item.key(), replaceList[item.key()].get<std::string>());
    }

    inFile.close();
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
    if (runtime->GetLanguage() == AbilityRuntime::Runtime::Language::STS) {
        TAG_LOGD(AAFwkTag::EXT, "etsruntime");
        SetExtensionEtsCheckCallback(runtime);
    } else {
        TAG_LOGD(AAFwkTag::EXT, "non ets runtime");
        auto moduleChecker = std::make_shared<AppModuleChecker>(extensionType_, extensionBlocklist_);
        runtime->SetModuleLoadChecker(moduleChecker);
        extensionBlocklist_.clear();
    }
}

void ExtensionConfigMgr::GenerateExtensionEtsBlocklists()
{
    if (!extensionEtsBlocklist_.empty()) {
        TAG_LOGD(AAFwkTag::EXT, "extension ets blocklist not empty");
        return;
    }
    auto iter = extensionBlocklist_.find(extensionType_);
    if (iter == extensionBlocklist_.end()) {
        TAG_LOGD(AAFwkTag::EXT, "null extension block, extensionType_: %{public}d", extensionType_);
        return;
    }
    for (const auto& module: iter->second) {
        auto replaceIter = replaceModuleListConfig_.find(module);
        if (replaceIter == replaceModuleListConfig_.end()) {
            std::string lowermodule;
            lowermodule.reserve(module.size());
            std::transform(module.cbegin(), module.cend(), std::back_inserter(lowermodule),
                [](unsigned char c) { return std::tolower(c); });
            extensionEtsBlocklist_.emplace(std::move(lowermodule));
        } else {
            std::string lowerReplacedModule;
            lowerReplacedModule.reserve(replaceIter->second.size());
            std::transform(replaceIter->second.cbegin(), replaceIter->second.cend(),
                std::back_inserter(lowerReplacedModule),
                [](unsigned char c) { return std::tolower(c); });
            extensionEtsBlocklist_.emplace(std::move(lowerReplacedModule));
        }
    }
}

bool ExtensionConfigMgr::CheckEtsModuleLoadable(const std::string &className)
{
    TAG_LOGD(AAFwkTag::EXT, "called");
    std::string classNameAfterRemovePreFix;
    if (className.length() >= ExtensionConfigItem::ITEM_LENGTH_OHOSPREFIX &&
        (className.compare(0, ExtensionConfigItem::ITEM_LENGTH_OHOSPREFIX, ExtensionConfigItem::ITEM_NAME_OHOSPREFIX)
        == 0)) {
        classNameAfterRemovePreFix = className.substr(ExtensionConfigItem::ITEM_LENGTH_OHOSPREFIX);
    } else if (className.length() >= ExtensionConfigItem::ITEM_LENGTH_HMSPREFIX &&
        (className.compare(0, ExtensionConfigItem::ITEM_LENGTH_HMSPREFIX, ExtensionConfigItem::ITEM_NAME_HMSPREFIX)
        == 0)) {
        classNameAfterRemovePreFix = className.substr(ExtensionConfigItem::ITEM_LENGTH_HMSPREFIX);
    } else {
        classNameAfterRemovePreFix = className;
    }
    std::string lowerClassName;
    lowerClassName.reserve(classNameAfterRemovePreFix.size());
    std::transform(classNameAfterRemovePreFix.cbegin(), classNameAfterRemovePreFix.cend(),
        std::back_inserter(lowerClassName), [](unsigned char c) { return std::tolower(c); });
    for (const auto& module: extensionEtsBlocklist_) {
        if (lowerClassName.compare(0, module.length(), module) == 0) {
            TAG_LOGD(AAFwkTag::EXT, "class is in block list: %{public}s", className.c_str());
            return false;
        }
    }
    return true;
}

void ExtensionConfigMgr::SetExtensionEtsCheckCallback(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::EXT, "extensionType_: %{public}d", extensionType_);
    auto callback = [extensionConfigMgrWeak = weak_from_this()](const std::string& className) -> bool {
        auto extensionConfigMgr = extensionConfigMgrWeak.lock();
        if (extensionConfigMgr == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null extensionConfigMgr");
            return false;
        }
        return extensionConfigMgr->CheckEtsModuleLoadable(className);
    };
    runtime->SetExtensionApiCheckCallback(callback);
}
}