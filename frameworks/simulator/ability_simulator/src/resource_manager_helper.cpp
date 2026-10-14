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

#include "resource_manager_helper.h"

#include "hilog_tag_wrapper.h"
#include "js_data_converter.h"

namespace OHOS {
namespace AbilityRuntime {
const std::string RESOURCES_DIR_NAME = "resources";
const std::string RESOURCES_FILE_NAME = "resources.index";

#if defined(WINDOWS_PLATFORM)
constexpr char DELIMITER[] = "\\";
#elif defined(MAC_PLATFORM)
constexpr char DELIMITER[] = "/";
#elif defined(LINUX_PLATFORM)
constexpr char DELIMITER[] = "/";
#else
#error "Unsupported platform"
#endif

ResourceManagerHelper &ResourceManagerHelper::GetInstance()
{
    TAG_LOGI(AAFwkTag::ABILITY_SIM, "GetInstance enter");
    static ResourceManagerHelper instance;
    TAG_LOGI(AAFwkTag::ABILITY_SIM, "GetInstance exit, instance initialized");
    return instance;
}

void ResourceManagerHelper::Init(const Options &options)
{
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Init enter, language:%{public}s, script:%{public}s, region:%{public}s, "
        "systemResourcePath:%{public}s, containerSdkPath:%{public}s, themeId:%{public}u, "
        "deviceType:%{public}d, orientation:%{public}d, density:%{public}f, colorMode:%{public}d",
        options.language.c_str(), options.script.c_str(), options.region.c_str(),
        options.systemResourcePath.c_str(), options.containerSdkPath.c_str(), options.themeId,
        static_cast<int32_t>(options.deviceConfig.deviceType),
        static_cast<int32_t>(options.deviceConfig.orientation), options.deviceConfig.density,
        static_cast<int32_t>(options.deviceConfig.colorMode));
    options_ = options;
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Init exit, options_ copied, language:%{public}s, script:%{public}s, region:%{public}s, "
        "systemResourcePath:%{public}s, containerSdkPath:%{public}s, themeId:%{public}u",
        options_.language.c_str(), options_.script.c_str(), options_.region.c_str(),
        options_.systemResourcePath.c_str(), options_.containerSdkPath.c_str(), options_.themeId);
}

void ResourceManagerHelper::GetResConfig(Global::Resource::ResConfig &resConfig, bool isCreateModuleContext)
{
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Global::Resource::ResConfig::SetLocaleInfo input, language:%{public}s, script:%{public}s, "
        "region:%{public}s",
        options_.language.c_str(), options_.script.c_str(), options_.region.c_str());
    resConfig.SetLocaleInfo(options_.language.c_str(), options_.script.c_str(), options_.region.c_str());
    TAG_LOGI(AAFwkTag::ABILITY_SIM, "Global::Resource::ResConfig::SetLocaleInfo completed");

    auto inputDeviceType = static_cast<int32_t>(options_.deviceConfig.deviceType);
    auto deviceType = ConvertDeviceType(options_.deviceConfig.deviceType);
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "ConvertDeviceType input:%{public}d, output:%{public}d", inputDeviceType,
        static_cast<int32_t>(deviceType));

    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Global::Resource::ResConfig::SetDeviceType input:%{public}d", static_cast<int32_t>(deviceType));
    resConfig.SetDeviceType(deviceType);

    if (!isCreateModuleContext) {
        auto inputOrientation = static_cast<int32_t>(options_.deviceConfig.orientation);
        auto direction = ConvertDirection(options_.deviceConfig.orientation);
        TAG_LOGI(AAFwkTag::ABILITY_SIM,
            "ConvertDirection input:%{public}d, output:%{public}d", inputOrientation,
            static_cast<int32_t>(direction));

        TAG_LOGI(AAFwkTag::ABILITY_SIM,
            "Global::Resource::ResConfig::SetDirection input:%{public}d", static_cast<int32_t>(direction));
        resConfig.SetDirection(direction);

        TAG_LOGI(AAFwkTag::ABILITY_SIM,
            "Global::Resource::ResConfig::SetScreenDensity input:%{public}f", options_.deviceConfig.density);
        resConfig.SetScreenDensity(options_.deviceConfig.density);
        TAG_LOGI(AAFwkTag::ABILITY_SIM, "Global::Resource::ResConfig::SetScreenDensity completed");

        auto inputColorMode = static_cast<int32_t>(options_.deviceConfig.colorMode);
        auto colorMode = ConvertColorMode(options_.deviceConfig.colorMode);

        TAG_LOGI(AAFwkTag::ABILITY_SIM,
            "Global::Resource::ResConfig::SetColorMode input:%{public}d", static_cast<int32_t>(colorMode));
        resConfig.SetColorMode(colorMode);

        TAG_LOGI(AAFwkTag::ABILITY_SIM,
            "Global::Resource::ResConfig::SetThemeId input:%{public}u", options_.themeId);
        resConfig.SetThemeId(options_.themeId);
    }
    TAG_LOGI(AAFwkTag::ABILITY_SIM, "GetResConfig exit");
}

void ResourceManagerHelper::AddSystemResource(std::shared_ptr<Global::Resource::ResourceManager> &resMgr)
{
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "AddSystemResource enter, resMgrIsNull:%{public}d, systemResourcePath:%{public}s, "
        "containerSdkPath:%{public}s",
        resMgr == nullptr, options_.systemResourcePath.c_str(), options_.containerSdkPath.c_str());

    std::string sysResIndexPath = options_.systemResourcePath + DELIMITER + RESOURCES_FILE_NAME;
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Global::Resource::ResourceManager::AddResource input, systemResourceIndexPath:%{public}s",
        sysResIndexPath.c_str());
    bool sysResourceAdded = resMgr->AddResource(sysResIndexPath.c_str());
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Global::Resource::ResourceManager::AddResource output, systemResourceIndexPath:%{public}s, "
        "result:%{public}d",
        sysResIndexPath.c_str(), sysResourceAdded);
    if (!sysResourceAdded) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Add system resource failed");
    }
    std::string hmsResIndexPath = options_.containerSdkPath + DELIMITER + RESOURCES_DIR_NAME + DELIMITER +
        RESOURCES_DIR_NAME + DELIMITER + RESOURCES_FILE_NAME;
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Global::Resource::ResourceManager::AddResource input, hmsResourceIndexPath:%{public}s",
        hmsResIndexPath.c_str());
    bool hmsResourceAdded = resMgr->AddResource(hmsResIndexPath.c_str());
    TAG_LOGI(AAFwkTag::ABILITY_SIM,
        "Global::Resource::ResourceManager::AddResource output, hmsResourceIndexPath:%{public}s, "
        "result:%{public}d",
        hmsResIndexPath.c_str(), hmsResourceAdded);
    if (!hmsResourceAdded) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Add hms resource failed");
    }
    TAG_LOGI(AAFwkTag::ABILITY_SIM, "AddSystemResource exit");
}
} // namespace AbilityRuntime
} // namespace OHOS
