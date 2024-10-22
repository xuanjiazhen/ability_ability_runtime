/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_CONTAINER_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_CONTAINER_H

#include <memory>
#include "ability_info.h"
#include "ability_manager_interface.h"
#include "bundle_mgr_interface.h"
#include "dummy_hap_module_info.h"
#include "hap_module_info.h"
#include "process_info.h"
#include "resource_manager.h"
#include "task/task_priority.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelper;
class AppContext;
using Want = OHOS::AAFwk::Want;
using AbilityStartSetting = AAFwk::AbilityStartSetting;
using PermissionRequestTask = std::function<void(const std::vector<std::string>&, const std::vector<int>&)>;
// Request permissions for user
#define OHOS_REQUEST_PERMISSION_BUNDLENAME "com.ohos.systemui"
#define OHOS_REQUEST_PERMISSION_ABILITY_NAME "com.ohos.systemui.systemdialog.MainAbility"

#define OHOS_REQUEST_PERMISSION_KEY "OHOS_REQUEST_PERMISSION_KEY"
#define OHOS_REQUEST_PERMISSIONS_LIST "OHOS_REQUEST_PERMISSIONS_LIST"
#define OHOS_REQUEST_PERMISSIONS_DES_LIST "OHOS_REQUEST_PERMISSIONS_DES_LIST"
#define OHOS_REQUEST_CALLER_BUNDLERNAME "OHOS_REQUEST_CALLER_BUNDLERNAME"

#define OHOS_RESULT_PERMISSION_KEY "OHOS_RESULT_PERMISSION_KEY"
#define OHOS_RESULT_PERMISSIONS_LIST "OHOS_RESULT_PERMISSIONS_LIST"
#define OHOS_RESULT_PERMISSIONS_LIST_YES "OHOS_RESULT_PERMISSIONS_LIST_YES"
#define OHOS_RESULT_PERMISSIONS_LIST_NO "OHOS_RESULT_PERMISSIONS_LIST_NO"
#define OHOS_RESULT_CALLER_BUNDLERNAME "OHOS_RESULT_CALLER_BUNDLERNAME"

#define OHOS_REQUEST_PERMISSION_VALUE 1

constexpr int DEFAULT_ACCOUNT_ID = -1;
class Context {
public:
    Context() = default;
    virtual ~Context() = default;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     *
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want, int requestCode)
    {
        return 0;
    }

    /**
     * @brief Destroys the current ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode TerminateAbility()
    {
        return 0;
    }

    /**
     * @brief Query whether the application of the specified PID and UID has been granted a certain permission
     *
     * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
     * @param pid Process id
     * @param uid
     * @return Returns 0 (IBundleManager.PERMISSION_GRANTED) if the current process has the permission;
     * returns -1 (IBundleManager.PERMISSION_DENIED) otherwise.
     */
    virtual int VerifyPermission(const std::string &permission, int pid, int uid)
    {
        return 0;
    }

    /**
     * @brief Obtains the bundle name of the ability that called the current ability.
     * You can use the obtained bundle name to check whether the calling ability is allowed to receive the data you will
     * send. If you did not use Ability.startAbilityForResult(ohos.aafwk.content.Want, int,
     * ohos.aafwk.ability.startsetting.AbilityStartSetting) to start the calling ability, null is returned.
     *
     * @return Returns the bundle name of the calling ability; returns null if no calling ability is available.
     */
    virtual std::string GetCallingBundle()
    {
        return 0;
    }

    /**
     * @brief Requests certain permissions from the system.
     * This method is called for permission request. This is an asynchronous method. When it is executed,
     * the task will be called back.
     *
     * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
     * @param permissionsState Indicates the list of permissions' state to be requested. This parameter cannot be null.
     * @param task The callback or promise fo js interface.
     */
    virtual void RequestPermissionsFromUser(std::vector<std::string> &permissions, std::vector<int> &permissionsState,
        PermissionRequestTask &&task)
    {}

    /**
     * @brief Starts a new ability with special ability start setting.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param abilityStartSetting Indicates the special start setting used in starting ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting)
    {
        return 0;
    }

    /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return True means success and false means failure
     */
    virtual bool ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
    {
        return false;
    }

    /**
     * @brief Disconnects the current ability from an ability
     *
     * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
     *              is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
    {
        return 0;
    }

    /**
     * @brief Starts multiple abilities.
     *
     * @param wants Indicates the Want containing information array about the target ability to start.
     */
    virtual void StartAbilities(const std::vector<AAFwk::Want> &wants) {}

    /**
     * @brief Checks whether the configuration of this ability is changing.
     *
     * @return Returns true if the configuration of this ability is changing and false otherwise.
     */
    virtual bool IsUpdatingConfigurations()
    {
        return false;
    }

    /**
     * @brief Informs the system of the time required for drawing this Page ability.
     *
     * @return Returns the notification is successful or fail
     */
    virtual bool PrintDrawnCompleted()
    {
        return false;
    }

    virtual sptr<IRemoteObject> GetToken()
    {
        return nullptr;
    }

    /**
     * Called when getting the ProcessInfo
     *
     * @return ProcessInfo
     */
    std::shared_ptr<ProcessInfo> GetProcessInfo() const;

    /**
     * Called when setting the ProcessInfo
     *
     * @param info ProcessInfo instance
     */
    void SetProcessInfo(const std::shared_ptr<ProcessInfo> &info);

    /**
     * @brief Obtains information about the current application. The returned application information includes basic
     * information such as the application name and application permissions.
     *
     * @return Returns the ApplicationInfo for the current application.
     */
    std::shared_ptr<ApplicationInfo> GetApplicationInfo() const;

    /**
     * @brief Set ApplicationInfo
     *
     * @param info ApplicationInfo instance.
     */
    void SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &info);

    /**
     * @brief Obtains a resource manager.
     *
     * @return Returns a ResourceManager object.
     */
    virtual std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager();

    /**
     * @brief Obtains the bundle name of the current ability.
     *
     * @return Returns the bundle name of the current ability.
     */
    std::string GetBundleName() const;

    /**
     * @brief Obtains the type of this application.
     *
     * @return Returns system if this application is a system application;
     * returns normal if it is released in OHOS AppGallery;
     * returns other if it is released by a third-party vendor;
     * returns an empty string if the query fails.
     */
    std::string GetAppType();

    /**
     * @brief Obtains the name of the current process.
     *
     * @return Returns the current process name.
     */
    std::string GetProcessName();

    /**
     * @brief Creates a Context object for an application with the given bundle name.
     *
     * @param bundleName Indicates the bundle name of the application.
     * @param flag  Indicates the flag for creating a Context object. It can be 0, any of
     * the following values, or any combination of the following values: CONTEXT_IGNORE_SECURITY,
     * CONTEXT_INCLUDE_CODE, and CONTEXT_RESTRICTED. The value 0 indicates that there is no restriction
     * on creating contexts for applications.
     * @param accountId Indicates the account id.
     *
     * @return Returns a Context object created for the specified application.
     */
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName, int flag,
        int accountId = DEFAULT_ACCOUNT_ID);

    /**
     * @brief Obtains information about the caller of this ability.
     *
     * @return Returns the caller information.
     */
    Uri GetCaller();

    /**
     * @brief SetUriString
     *
     * @param uri the uri to set.
     */
    void SetUriString(const std::string &uri);

    /**
     * @brief InitResourceManager
     *
     * @param bundleInfo  BundleInfo
     */
    void InitResourceManager(BundleInfo &bundleInfo, std::shared_ptr<Context> context);

    /**
     * @brief set resourceManager 
     *
     */
    void SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);

    /**
     * @brief Obtains the lifecycle state info.
     *
     * @return Returns the lifecycle state info.
     */
    AAFwk::LifeCycleStateInfo GetLifeCycleStateInfo() const;

    /**
     * @brief Set the LifeCycleStateInfo
     *
     * @param info the info to set.
     */
    void SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info);

    /**
     * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
     *  source code, and configuration files of a module.
     *
     * @return Returns the path of the package file.
     */
    std::string GetBundleCodePath();

    virtual std::shared_ptr<Context> GetApplicationContext()
    {
        return nullptr;
    }
private:
    void LoadResources(BundleInfo &bundleInfo, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
        std::unique_ptr<Global::Resource::ResConfig> &resConfig, std::shared_ptr<Context> &context);

    std::shared_ptr<ProcessInfo> processInfo_ = nullptr;
    std::shared_ptr<ApplicationInfo> applicationInfo_ = nullptr;
    AAFwk::LifeCycleStateInfo lifeCycleStateInfo_;
    std::string uriString_ = "";
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_CONTAINER_H
