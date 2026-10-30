/**
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
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <gtest/gtest.h>
#include <mutex>
#include <signal.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_mgr_client.h"
#include "bundle_mgr_helper.h"
#include "bundle_mgr_interface.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "parameters.h"
#include "status_receiver_host.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace std::chrono_literals;
namespace OHOS::AAFwk {
namespace {
constexpr const char *CALLER = "com.ohos.appservice.proxy.caller";
constexpr const char *DENIED = "com.ohos.appservice.proxy.denied";
constexpr const char *PROVIDER = "com.ohos.appservice.proxy.provider";
constexpr const char *UNLICENSED = "com.ohos.appservice.proxy.unlicensed";
constexpr const char *COMPONENT_PERMISSION = "ohos.permission.SUPPORT_APP_SERVICE_EXTENSION_ADVANCED";
constexpr int32_t USER_UID_RANGE = 200000;
constexpr uid_t EDM_UID = 3057;
constexpr auto WAIT_TIMEOUT = 15s;

} // namespace

class AppServiceTestConnection final : public AbilityConnectionStub {
public:
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &object, int result) override
    {
        std::lock_guard lock(mutex_);
        object_ = object;
        result_ = result;
        done_ = true;
        cv_.notify_all();
    }
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int result) override {}
    sptr<IRemoteObject> WaitForObject()
    {
        std::unique_lock lock(mutex_);
        if (!cv_.wait_for(lock, WAIT_TIMEOUT, [this] { return done_; }) || result_ != ERR_OK) {
            return nullptr;
        }
        return object_;
    }
    bool HasObject()
    {
        std::lock_guard lock(mutex_);
        return object_ != nullptr;
    }
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool done_ = false;
    int result_ = -1;
    sptr<IRemoteObject> object_;
};

class AppServiceInstallReceiver final : public AppExecFwk::StatusReceiverHost {
public:
    void OnStatusNotify(int progress) override {}
    void OnFinished(int32_t resultCode, const std::string &resultMsg) override
    {
        std::lock_guard lock(mutex_);
        result_ = resultCode;
        done_ = true;
        cv_.notify_all();
    }
    int32_t WaitForResult()
    {
        std::unique_lock lock(mutex_);
        return cv_.wait_for(lock, 45s, [this] { return done_; }) ? result_ : -1;
    }
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool done_ = false;
    int32_t result_ = -1;
};

namespace {
int RunBundleCommand(const std::string &path, int32_t userId, bool install = true)
{
    const std::string user = std::to_string(userId);
    pid_t child = fork();
    if (child == 0) {
        // bm changes SELinux domain on exec and must load its system libraries.
        unsetenv("LD_LIBRARY_PATH");
        if (install) {
            execlp("bm", "bm", "install", "-p", path.c_str(), "-u", user.c_str(), nullptr);
        } else {
            execlp("bm", "bm", "uninstall", "-n", path.c_str(), "-u", user.c_str(), nullptr);
        }
        _exit(127);
    }
    if (child < 0) {
        return -1;
    }
    int status = 0;
    auto deadline = std::chrono::steady_clock::now() + 45s;
    while (std::chrono::steady_clock::now() < deadline) {
        if (waitpid(child, &status, WNOHANG) == child) {
            return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        }
        std::this_thread::sleep_for(100ms);
    }
    kill(child, SIGKILL);
    waitpid(child, &status, 0);
    return -1;
}
}

class AppServiceProxyModuleTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        ASSERT_EQ(0U, geteuid()) << "The test must be able to assume the EDM uid";
        suiteToken_ = GetSelfTokenID();
        const char *permissions[] = {
            "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED", "ohos.permission.GET_SENSITIVE_PERMISSIONS",
            "ohos.permission.GET_INSTALLED_BUNDLE_LIST", "ohos.permission.CLEAN_BACKGROUND_PROCESSES",
            "ohos.permission.GET_RUNNING_INFO", "ohos.permission.MANAGE_LOCAL_ACCOUNTS",
            "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS", "ohos.permission.INSTALL_BUNDLE",
            "ohos.permission.GET_LOCAL_ACCOUNTS", "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS_EXTENSION",
            "ohos.permission.INSTALL_ENTERPRISE_MDM_BUNDLE", "ohos.permission.UNINSTALL_BUNDLE"
        };
        NativeTokenInfoParams token = {};
        token.processName = "app_service_proxy_test";
        token.aplStr = "system_core";
        token.perms = permissions;
        token.permsNum = sizeof(permissions) / sizeof(permissions[0]);
        auto testToken = GetAccessTokenId(&token);
        ASSERT_NE(0U, testToken);
        ASSERT_EQ(0, SetSelfTokenID(testToken));
        ASSERT_EQ(0, Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo());
        ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(suiteUser_));
        auto bms = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
        ASSERT_NE(nullptr, bms);
        const char *resource = std::getenv("APP_SERVICE_TEST_HAP_DIR");
        const std::string hapDir = resource == nullptr ? "/data/test/app_service/resource" : resource;
        for (const auto &name : {"caller", "denied", "provider", "unlicensed"}) {
            const std::string bundle = std::string("com.ohos.appservice.proxy.") + name;
            AppExecFwk::ApplicationInfo existing;
            ASSERT_FALSE(bms->GetApplicationInfoWithAppIndex(bundle, 0, suiteUser_, existing))
                << "Refusing to replace pre-existing test package: " << bundle;
            ASSERT_EQ(0, RunBundleCommand(hapDir + "/app_service_" + name + ".hap", suiteUser_));
            ASSERT_TRUE(bms->GetApplicationInfoWithAppIndex(bundle, 0, suiteUser_, existing))
                << "Test package installation failed: " << bundle;
            installed_.push_back(bundle);
        }
    }
    static void TearDownTestCase()
    {
        for (const auto &bundle : installed_) {
            EXPECT_EQ(0, RunBundleCommand(bundle, suiteUser_, false));
        }
        EXPECT_EQ(0, SetSelfTokenID(suiteToken_));
    }
    void SetUp() override
    {
        originalUid_ = geteuid();
        ASSERT_TRUE(system::GetBoolParameter("const.abilityms.support_app_service", false))
            << "AppService capability is absent from the test image";
        ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId_));
        bms_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
        ASSERT_NE(nullptr, bms_);
        ASSERT_TRUE(bms_->GetApplicationInfoWithAppIndex(CALLER, 0, userId_, caller_));
        ASSERT_TRUE(bms_->GetApplicationInfoWithAppIndex(DENIED, 0, userId_, denied_));
        AppExecFwk::ApplicationInfo provider;
        ASSERT_TRUE(bms_->GetApplicationInfoWithAppIndex(PROVIDER, 0, userId_, provider));
        ASSERT_EQ(Security::AccessToken::PERMISSION_GRANTED,
            Security::AccessToken::AccessTokenKit::VerifyAccessToken(provider.accessTokenId, COMPONENT_PERMISSION));
        ASSERT_TRUE(StopTarget(PROVIDER, userId_));
        ASSERT_TRUE(StopTarget(UNLICENSED, userId_));
        ASSERT_EQ(0, seteuid(EDM_UID));
    }
    void TearDown() override
    {
        for (const auto &connection : connections_) {
            AbilityManagerClient::GetInstance()->DisconnectAbility(connection);
        }
        connections_.clear();
        ASSERT_EQ(0, seteuid(originalUid_));
        if (workUser_ >= 0) {
            const auto restored = AccountSA::OsAccountManager::ActivateOsAccount(userId_);
            EXPECT_EQ(ERR_OK, restored);
            if (restored == ERR_OK) {
                EXPECT_EQ(ERR_OK, AccountSA::OsAccountManager::RemoveOsAccount(workUser_));
            }
        }
    }
    bool StopTarget(const std::string &bundle, int32_t userId)
    {
        AbilityManagerClient::GetInstance()->KillProcessWithUserId(bundle, true, -1, "AppServiceTest", userId);
        AppExecFwk::AppMgrClient client;
        auto deadline = std::chrono::steady_clock::now() + WAIT_TIMEOUT;
        while (std::chrono::steady_clock::now() < deadline) {
            std::vector<AppExecFwk::RunningProcessInfo> processes;
            if (client.GetProcessRunningInfosByUserId(processes, userId) != AppExecFwk::AppMgrResultCode::RESULT_OK) {
                return false;
            }
            bool running = std::any_of(processes.begin(), processes.end(), [&bundle](const auto &process) {
                return std::find(process.bundleNames.begin(), process.bundleNames.end(), bundle) !=
                    process.bundleNames.end();
            });
            if (!running) {
                return true;
            }
            std::this_thread::sleep_for(100ms);
        }
        return false;
    }
    int Connect(const AppExecFwk::ApplicationInfo &caller, const std::string &bundle,
        const std::string &component, int32_t userId, sptr<AppServiceTestConnection> &connection)
    {
        Want want;
        want.SetElementName(bundle, component);
        auto identity = std::make_shared<IndirectCallerInfo>();
        identity->tokenId = caller.accessTokenId;
        identity->callerUid = caller.uid;
        identity->callerPid = getpid();
        connection = sptr<AppServiceTestConnection>::MakeSptr();
        connections_.push_back(connection);
        return AbilityManagerClient::GetInstance()->ConnectAbilityWithIndirectCallerInfo(
            want, connection, nullptr, userId, AppExecFwk::ExtensionAbilityType::APP_SERVICE, identity);
    }
    void ExpectEcho(const sptr<AppServiceTestConnection> &connection, int32_t userId)
    {
        auto remote = connection->WaitForObject();
        ASSERT_NE(nullptr, remote) << "No successful connection callback/object";
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        ASSERT_TRUE(data.WriteInterfaceToken(u"ohos.appservice.proxy.test"));
        const std::string message = "echo-" + std::to_string(getpid()) + "-" + std::to_string(sequence_++);
        ASSERT_TRUE(data.WriteString16(Str8ToStr16(message)));
        ASSERT_EQ(ERR_OK, remote->SendRequest(1, data, reply, option));
        EXPECT_EQ(message, Str16ToStr8(reply.ReadString16()));
        int32_t uid = -1;
        ASSERT_TRUE(reply.ReadInt32(uid));
        EXPECT_EQ(userId, uid / USER_UID_RANGE);
    }
    inline static uint64_t suiteToken_ = 0;
    inline static int32_t suiteUser_ = -1;
    inline static std::vector<std::string> installed_;
    uid_t originalUid_ = 0;
    int32_t userId_ = -1;
    int32_t workUser_ = -1;
    uint32_t sequence_ = 0;
    AppExecFwk::ApplicationInfo caller_;
    AppExecFwk::ApplicationInfo denied_;
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bms_;
    std::vector<sptr<AppServiceTestConnection>> connections_;
};

/** @tc.name: Connect_001
 * @tc.desc: A valid proxy cold-loads a licensed AppService and receives an RPC reply.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyModuleTest, Connect_001, TestSize.Level1)
{
    sptr<AppServiceTestConnection> connection;
    ASSERT_EQ(ERR_OK, Connect(caller_, PROVIDER, "Service", userId_, connection));
    ExpectEcho(connection, userId_);
}

/** @tc.name: ComponentPermission_001
 * @tc.desc: The native proxy cannot connect a target lacking the integration license.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyModuleTest, ComponentPermission_001, TestSize.Level1)
{
    sptr<AppServiceTestConnection> connection;
    EXPECT_EQ(TARGET_ABILITY_NOT_SERVICE, Connect(caller_, UNLICENSED, "Service", userId_, connection));
    EXPECT_FALSE(connection->HasObject());
}

/** @tc.name: CallerPermission_001
 * @tc.desc: Static access permission is checked against A, not the proxy SA.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyModuleTest, CallerPermission_001, TestSize.Level1)
{
    sptr<AppServiceTestConnection> connection;
    EXPECT_EQ(ERR_STATIC_CFG_PERMISSION, Connect(denied_, PROVIDER, "ProtectedService", userId_, connection));
    EXPECT_FALSE(connection->HasObject());
    ASSERT_EQ(ERR_OK, Connect(caller_, PROVIDER, "ProtectedService", userId_, connection));
    ExpectEcho(connection, userId_);
}

/** @tc.name: Visibility_001
 * @tc.desc: A cannot connect another application's non-exported component through an SA.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyModuleTest, Visibility_001, TestSize.Level1)
{
    sptr<AppServiceTestConnection> connection;
    EXPECT_EQ(ABILITY_VISIBLE_FALSE_DENY_REQUEST, Connect(caller_, PROVIDER, "HiddenService", userId_, connection));
    EXPECT_FALSE(connection->HasObject());
}

/** @tc.name: ColdAndActive_001
 * @tc.desc: The allowlist restricts cold loading, while an active component permits otherwise-authorized A.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyModuleTest, ColdAndActive_001, TestSize.Level1)
{
    sptr<AppServiceTestConnection> denied;
    EXPECT_EQ(ERR_TARGET_NOT_STARTED, Connect(denied_, PROVIDER, "RestrictedService", userId_, denied));
    EXPECT_FALSE(denied->HasObject());
    sptr<AppServiceTestConnection> allowed;
    ASSERT_EQ(ERR_OK, Connect(caller_, PROVIDER, "RestrictedService", userId_, allowed));
    ExpectEcho(allowed, userId_);
    ASSERT_EQ(ERR_OK, Connect(denied_, PROVIDER, "RestrictedService", userId_, denied));
    ExpectEcho(denied, userId_);
}

/** @tc.name: BackgroundUi_001
 * @tc.desc: A background AppService gets no UI-start exemption without full capability.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyModuleTest, BackgroundUi_001, TestSize.Level1)
{
    ASSERT_TRUE(StopTarget(CALLER, userId_));
    sptr<AppServiceTestConnection> connection;
    ASSERT_EQ(ERR_OK, Connect(caller_, PROVIDER, "Service", userId_, connection));
    auto remote = connection->WaitForObject();
    ASSERT_NE(nullptr, remote);
    // Let the process-start grace period expire; neither test HAP has a foreground UI.
    std::this_thread::sleep_for(2s);
    MessageParcel start;
    MessageParcel startReply;
    MessageOption option;
    ASSERT_TRUE(start.WriteInterfaceToken(u"ohos.appservice.proxy.test"));
    ASSERT_EQ(ERR_OK, remote->SendRequest(2, start, startReply, option));
    int32_t acknowledged = -1;
    ASSERT_TRUE(startReply.ReadInt32(acknowledged));
    ASSERT_EQ(0, acknowledged);
    int32_t result = -1;
    auto deadline = std::chrono::steady_clock::now() + WAIT_TIMEOUT;
    while (result == -1 && std::chrono::steady_clock::now() < deadline) {
        MessageParcel request;
        MessageParcel reply;
        ASSERT_TRUE(request.WriteInterfaceToken(u"ohos.appservice.proxy.test"));
        ASSERT_EQ(ERR_OK, remote->SendRequest(3, request, reply, option));
        ASSERT_TRUE(reply.ReadInt32(result));
        if (result == -1) {
            std::this_thread::sleep_for(100ms);
        }
    }
    const bool fullCapability = system::GetBoolParameter("const.abilityms.app_service_full_capability", false);
    EXPECT_EQ(fullCapability ? 0 : 201, result) << "Public startAbility Promise result";
    ASSERT_TRUE(StopTarget(CALLER, userId_));
}

/** @tc.name: BackgroundUser_001
 * @tc.desc: The proxy connects the work-user instance while a same-package foreground-user instance exists.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyModuleTest, BackgroundUser_001, TestSize.Level1)
{
    AccountSA::OsAccountInfo account;
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::CreateOsAccount(
        "AppServiceProxyTest", AccountSA::OsAccountType::NORMAL, account));
    workUser_ = account.GetLocalId();
    ASSERT_NE(userId_, workUser_);
    // StartOsAccount is a no-op on this branch. Initialize the user's real AMS
    // managers by activation, then return to the original user before connecting.
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::ActivateOsAccount(workUser_));
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::ActivateOsAccount(userId_));
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::SetOsAccountIsVerified(workUser_, true));
    bool verified = false;
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::IsOsAccountVerified(workUser_, verified));
    ASSERT_TRUE(verified);
    const char *resource = std::getenv("APP_SERVICE_TEST_HAP_DIR");
    const std::string hapDir = resource == nullptr ? "/data/test/app_service/resource" : resource;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(nullptr, samgr);
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID));
    ASSERT_NE(nullptr, bundleMgr);
    auto installer = bundleMgr->GetBundleInstaller();
    ASSERT_NE(nullptr, installer);
    AppExecFwk::InstallParam params;
    params.userId = workUser_;
    params.installFlag = AppExecFwk::InstallFlag::REPLACE_EXISTING;
    auto receiver = sptr<AppServiceInstallReceiver>::MakeSptr();
    ASSERT_EQ(ERR_OK, installer->StreamInstall({hapDir + "/app_service_provider.hap"}, params, receiver));
    ASSERT_EQ(ERR_OK, receiver->WaitForResult());
    AppExecFwk::ApplicationInfo provider;
    ASSERT_TRUE(bms_->GetApplicationInfoWithAppIndex(PROVIDER, 0, workUser_, provider));
    int32_t foreground = -1;
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(foreground));
    ASSERT_EQ(userId_, foreground);
    sptr<AppServiceTestConnection> personal;
    ASSERT_EQ(ERR_OK, Connect(caller_, PROVIDER, "Service", userId_, personal));
    ExpectEcho(personal, userId_);
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(foreground));
    ASSERT_EQ(userId_, foreground);
    sptr<AppServiceTestConnection> work;
    ASSERT_EQ(ERR_OK, Connect(caller_, PROVIDER, "Service", workUser_, work));
    ExpectEcho(work, workUser_);
    ASSERT_EQ(ERR_OK, AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(foreground));
    EXPECT_EQ(userId_, foreground);
}
} // namespace OHOS::AAFwk
