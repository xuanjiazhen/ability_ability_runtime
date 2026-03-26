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

#ifndef OHOS_ABILITY_RUNTIME_TOOLS_AA_SCRIPT_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_TOOLS_AA_SCRIPT_CONTEXT_H

#include "ability_runtime/context/context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace AaTool {

class ScriptContext : public Context {
public:
    ScriptContext() = default;
    ~ScriptContext() override = default;

    // Context interface implementation
    std::string GetBundleName() const override
    {
        return bundleName_;
    }

    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override
    {
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override
    {
        return applicationInfo_;
    }

    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override
    {
        return resourceManager_;
    }

    std::string GetBundleCodePath() const override
    {
        return bundleCodePath_;
    }

    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override
    {
        return hapModuleInfo_;
    }

    std::string GetBundleCodeDir() override
    {
        return bundleCodeDir_;
    }

    std::string GetCacheDir() override
    {
        return cacheDir_;
    }

    std::string GetTempDir() override
    {
        return tempDir_;
    }

    std::string GetFilesDir() override
    {
        return filesDir_;
    }

    std::string GetResourceDir(const std::string &moduleName = "") override
    {
        return resourceDir_;
    }

    bool IsUpdatingConfigurations() override
    {
        return false;
    }

    bool PrintDrawnCompleted() override
    {
        return false;
    }

    std::string GetDatabaseDir() override
    {
        return databaseDir_;
    }

    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override
    {
        databaseDir = databaseDir_;
        return ERR_OK;
    }

    std::string GetPreferencesDir() override
    {
        return preferencesDir_;
    }

    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist,
        std::string &preferencesDir) override
    {
        preferencesDir = preferencesDir_;
        return ERR_OK;
    }

    std::string GetGroupDir(std::string groupId) override
    {
        return groupDir_;
    }

    std::string GetDistributedFilesDir() override
    {
        return distributedFilesDir_;
    }

    std::string GetCloudFileDir() override
    {
        return cloudFileDir_;
    }

    std::string GetLogFileDir() override
    {
        return logFileDir_;
    }

    sptr<IRemoteObject> GetToken() override
    {
        return nullptr;
    }

    void SetToken(const sptr<IRemoteObject> &token) override
    {}

    void SwitchArea(int mode) override
    {}

    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override
    {
        return nullptr;
    }

    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName,
        const std::string &moduleName) override
    {
        return nullptr;
    }

    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override
    {
        return nullptr;
    }

    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName,
        const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override
    {
        return ERR_INVALID_VALUE;
    }

    int GetArea() override
    {
        return 0;
    }

    std::string GetProcessName() override
    {
        return processName_;
    }

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override
    {
        return configuration_;
    }

    std::string GetBaseDir() const override
    {
        return baseDir_;
    }

    Global::Resource::DeviceType GetDeviceType() const override
    {
        return Global::Resource::DeviceType::DEVICE_PHONE;
    }

    std::shared_ptr<Context> CreateAreaModeContext(int areaMode) override
    {
        return nullptr;
    }

    std::shared_ptr<Context> CreateModuleOrPluginContext(const std::string &bundleName,
        const std::string &moduleName) override
    {
        return nullptr;
    }

#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<Context> CreateDisplayContext(uint64_t displayId) override
    {
        return nullptr;
    }
#endif

    // ScriptContext specific methods
    void SetBundleName(const std::string &bundleName)
    {
        bundleName_ = bundleName;
    }

    void SetProcessName(const std::string &processName)
    {
        processName_ = processName;
    }

    void SetBundleCodePath(const std::string &path)
    {
        bundleCodePath_ = path;
        bundleCodeDir_ = path;
    }

    void SetBaseDir(const std::string &dir)
    {
        baseDir_ = dir;
    }

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || contextTypeId == ScriptContext::SCRIPT_CONTEXT_TYPE_ID;
    }

private:
    using SelfType = ScriptContext;
    static const size_t SCRIPT_CONTEXT_TYPE_ID;

    std::string bundleName_ = "com.example.script";
    std::string processName_ = "com.example.script";
    std::string bundleCodePath_ = "";
    std::string bundleCodeDir_ = "";
    std::string cacheDir_ = "/tmp/cache";
    std::string tempDir_ = "/tmp/temp";
    std::string filesDir_ = "/tmp/files";
    std::string resourceDir_ = "";
    std::string databaseDir_ = "/tmp/database";
    std::string preferencesDir_ = "/tmp/preferences";
    std::string groupDir_ = "/tmp/group";
    std::string distributedFilesDir_ = "/tmp/distributed";
    std::string cloudFileDir_ = "/tmp/cloud";
    std::string logFileDir_ = "/tmp/log";
    std::string baseDir_ = "/tmp";

    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo_ =
        std::make_shared<AppExecFwk::ApplicationInfo>();
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager_;
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo_ =
        std::make_shared<AppExecFwk::HapModuleInfo>();
    std::shared_ptr<AppExecFwk::Configuration> configuration_ =
        std::make_shared<AppExecFwk::Configuration>();
};

inline const size_t ScriptContext::SCRIPT_CONTEXT_TYPE_ID = 0x1000;

// Define error codes for ScriptContext
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_INVALID_VALUE = -1;

}  // namespace AaTool
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_TOOLS_AA_SCRIPT_CONTEXT_H
