/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "fa_context.h"

#include <regex>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "app_context.h"
#include "bundle_constants.h"
#include "bundle_mgr_helper.h"
#include "constants.h"
#include "hilog_tag_wrapper.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
// for api7 demo special
constexpr int CURRENT_ACCOUNT_ID = 100;
const int32_t TYPE_RESERVE = 1;
const int32_t TYPE_OTHERS = 2;

std::shared_ptr<ProcessInfo> Context::GetProcessInfo() const
{
    return processInfo_;
}

void Context::SetProcessInfo(const std::shared_ptr<ProcessInfo> &info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "info is empty");
        return;
    }
    processInfo_ = info;
}

std::shared_ptr<ApplicationInfo> Context::GetApplicationInfo() const
{
    return applicationInfo_;
}

void Context::SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "info is empty");
        return;
    }
    applicationInfo_ = info;
}

std::shared_ptr<Global::Resource::ResourceManager> Context::GetResourceManager()
{
    return resourceManager_;
}

std::string Context::GetBundleName() const
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

std::string Context::GetAppType()
{
    auto ptr = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get bundle manager service.");
        return "";
    }
    std::string retString = ptr->GetAppType(applicationInfo_->bundleName);
    return retString;
}

std::string Context::GetProcessName()
{
    return (processInfo_ != nullptr) ? processInfo_->GetProcessName() : "";
}

std::shared_ptr<Context> Context::CreateBundleContext(const std::string &bundleName, int flag, int accountId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "The bundleName is empty");
        return nullptr;
    }

    if (strcmp(bundleName.c_str(), GetBundleName().c_str()) == 0) {
        return GetApplicationContext();
    }

    std::shared_ptr<BundleMgrHelper> bundleMgr = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "The bundleMgr is nullptr");
        return nullptr;
    }

    BundleInfo bundleInfo;
    TAG_LOGI(AAFwkTag::APPKIT, "Length: %{public}zu, bundleName: %{public}s, accountId is %{public}d",
        bundleName.length(),
        bundleName.c_str(),
        accountId);
    int realAccountId = CURRENT_ACCOUNT_ID;
    if (accountId != DEFAULT_ACCOUNT_ID) {
        realAccountId = accountId;
    }
    bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, realAccountId);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get Bundle Info");
        return nullptr;
    }

    std::shared_ptr<AppContext> appContext = std::make_shared<AppContext>();

    // init resourceManager.
    InitResourceManager(bundleInfo, appContext);
    appContext->SetApplicationInfo(std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo));
    return appContext;
}

void Context::InitResourceManager(BundleInfo &bundleInfo, std::shared_ptr<Context> context)
{
    TAG_LOGD(AAFwkTag::APPKIT, "InitResourceManager begin, bundleName:%{public}s, codePath:%{public}s",
        bundleInfo.name.c_str(), bundleInfo.applicationInfo.codePath.c_str());
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "InitResourceManager deal is nullptr");
        return;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    std::string moduleName;
    std::string hapPath;
    std::vector<std::string> overlayPaths;
    int32_t appType;
    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE)) {
        appType = TYPE_RESERVE;
    } else if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        appType = TYPE_OTHERS;
    } else {
        appType = 0;
    }
    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE) ||
        bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
            bundleInfo.name, moduleName, hapPath, overlayPaths, *resConfig, appType));
        if (resourceManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to create resourceManager");
            return;
        }
        context->SetResourceManager(resourceManager);
        return;
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleInfo.name, moduleName, hapPath, overlayPaths, *resConfig, appType));
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "create resourceManager failed");
        return;
    }
    LoadResources(bundleInfo, resourceManager, resConfig, context);
}

void Context::LoadResources(BundleInfo &bundleInfo,
    std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    std::unique_ptr<Global::Resource::ResConfig> &resConfig, std::shared_ptr<Context> &context)
{
    TAG_LOGD(AAFwkTag::APPKIT, "hapModuleInfos count: %{public}zu",
        bundleInfo.hapModuleInfos.size());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "resConfig is nullptr");
        return;
    }
    std::regex pattern(AbilityBase::Constants::ABS_CODE_PATH);
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        std::string loadPath;
        if (!hapModuleInfo.hapPath.empty()) {
            loadPath = hapModuleInfo.hapPath;
        } else {
            loadPath = hapModuleInfo.resourcePath;
        }
        if (loadPath.empty()) {
            continue;
        }
        loadPath = std::regex_replace(loadPath, pattern, AbilityBase::Constants::LOCAL_BUNDLES);
        TAG_LOGD(AAFwkTag::APPKIT, "loadPath: %{private}s", loadPath.c_str());
        if (!resourceManager->AddResource(loadPath.c_str())) {
            TAG_LOGE(AAFwkTag::APPKIT, "Context::InitResourceManager AddResource failed");
        }
    }

    resConfig->SetLocaleInfo("zh", "Hans", "CN");
#ifdef SUPPORT_GRAPHICS
    if (resConfig->GetLocaleInfo() != nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT,
            "language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(),
            resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "language: GetLocaleInfo is null.");
    }
#endif
    resourceManager->UpdateResConfig(*resConfig);
    context->SetResourceManager(resourceManager);
}

void Context::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    resourceManager_ = resourceManager;
}

Uri Context::GetCaller()
{
    Uri uri(uriString_);
    return uri;
}

void Context::SetUriString(const std::string &uri)
{
    uriString_ = uri;
}

void Context::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    lifeCycleStateInfo_ = info;
}

AAFwk::LifeCycleStateInfo Context::GetLifeCycleStateInfo() const
{
    return lifeCycleStateInfo_;
}

std::string Context::GetBundleCodePath()
{
    return AbilityBase::Constants::LOCAL_CODE_PATH;
}
}  // namespace AppExecFwk
}  // namespace OHOS
