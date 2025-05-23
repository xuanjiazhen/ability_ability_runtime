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

#ifndef MOCK_FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_APPLICATION_INFO_H
#define MOCK_FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_APPLICATION_INFO_H

#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
enum class MultiAppModeType : uint8_t {
    UNSPECIFIED = 0,
    MULTI_INSTANCE = 1,
    APP_CLONE = 2,
};

enum ApplicationFlag {
    GET_BASIC_APPLICATION_INFO = 0x00000000,
    GET_APPLICATION_INFO_WITH_PERMISSION = 0x00000008,
    GET_APPLICATION_INFO_WITH_METADATA = 0x00000040,
    GET_APPLICATION_INFO_WITH_DISABLE = 0x00000200,
    GET_APPLICATION_INFO_WITH_CERTIFICATE_FINGERPRINT = 0x00000400,
    GET_ALL_APPLICATION_INFO = 0xFFFF0000,
};

enum class BundleType {
    APP = 0,
    ATOMIC_SERVICE = 1,
    SHARED = 2,
};

struct Metadata {
    std::string name;
    std::string value;
    std::string resource;
};

struct CustomizeData {
    std::string name;
    std::string value;
    std::string extra;
};

struct MetaData {
    std::vector<CustomizeData> customizeData;
};

struct Resource {
    std::string bundleName;
    std::string moduleName;
    int32_t id = 0;
};

struct ApplicationInfo {
    std::string name;
    std::string bundleName;

    uint32_t versionCode = 0;
    std::string versionName;
    int32_t minCompatibleVersionCode = 0;

    uint32_t apiCompatibleVersion = 0;
    int32_t apiTargetVersion = 0;
    int64_t crowdtestDeadline = -1;

    std::string iconPath;
    int32_t iconId = 0;
    Resource iconResource;

    std::string label;
    int32_t labelId = 0;
    Resource labelResource;

    std::string description;
    int32_t descriptionId = 0;

    bool keepAlive = false;
    bool removable = true;
    bool singleton = false;
    bool userDataClearable = true;
    bool accessible = false;
    bool runningResourcesApply = false;
    bool associatedWakeUp = false;
    bool hideDesktopIcon = false;
    bool formVisibleNotify = false;
    std::vector<std::string> allowCommonEvent;
    std::vector<int32_t> resourcesApply;

    bool isSystemApp = false;
    bool isLauncherApp = false;
    bool isFreeInstallApp = false;
    bool asanEnabled = false;
    std::string asanLogPath;

    std::string codePath;
    std::string dataDir;
    std::string dataBaseDir;
    std::string cacheDir;
    std::string entryDir;

    std::string apiReleaseType;
    bool debug = false;
    std::string deviceId;
    bool distributedNotificationEnabled = true;
    std::string entityType = "";
    std::string process;
    int32_t supportedModes = 0;
    std::string vendor;
    std::string appPrivilegeLevel = "";
    std::string appDistributionType = "none";
    std::string appProvisionType = "release";
    uint32_t accessTokenId = 0;
    uint64_t accessTokenIdEx = 0;
    bool enabled = false;
    int32_t uid = -1;
    std::string nativeLibraryPath;
    std::string cpuAbi;
    std::string arkNativeFilePath;
    std::string arkNativeFileAbi;
    std::vector<std::string> permissions;
    std::vector<std::string> moduleSourceDirs;
    std::map<std::string, std::vector<CustomizeData>> metaData;
    std::map<std::string, std::vector<Metadata>> metadata;
    std::vector<std::string> targetBundleList;
    std::string fingerprint;
    std::string icon;
    int32_t flags = 0;
    std::string entryModuleName;
    bool isCompressNativeLibs = true;
    std::string signatureKey;
    bool multiProjects = false;
    bool needAppDetail = false;
    std::string appDetailAbilityLibraryPath;
    std::string targetBundleName;
    int32_t targetPriority;
    int32_t overlayState;
    bool split = true;
    BundleType bundleType = BundleType::APP;
    std::string compileSdkVersion;
    std::string compileSdkType = "";
    int32_t appIndex = 0;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif
