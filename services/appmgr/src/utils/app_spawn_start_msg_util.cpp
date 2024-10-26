#include "accesstoken_kit.h"
#include "app_mgr_constants.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "net_conn_client.h"
#include "os_account_manager_wrapper.h"
#include "ohos_account_kits.h"
# include "utils/app_spawn_start_msg_util.h"

namespace OHOS {
namespace AppExecFwk {

using namespace OHOS::Security;
#define APP_OVERLAY_FLAG 0x100
#define APP_ACCESS_BUNDLE_DIR 0x20

constexpr int32_t BASE_USER_RANGE = 200000;
constexpr const int32_t API15 = 15;
constexpr const int32_t API_VERSION_MOD = 100;
constexpr const char* RENDER_PARAM = "invalidparam";
constexpr const char* ISOLATED_SANDBOX = "isolatedSandbox";
constexpr const char* PERMISSION_ACCESS_BUNDLE_DIR = "ohos.permission.ACCESS_BUNDLE_DIR";
constexpr const char* PERMISSION_INTERNET = "ohos.permission.INTERNET";
#ifdef APP_MGR_SERVICE_APPMS
constexpr int32_t NETSYS_SOCKET_GROUPID = 1097;
#endif
constexpr const char* PERMISSION_MANAGE_VPN = "ohos.permission.MANAGE_VPN";
constexpr int32_t BLUETOOTH_GROUPID = 1002;
constexpr const char* TSAN_FLAG_NAME = "tsanEnabled";
constexpr const char* HWASAN_FLAG_NAME = "hwasanEnabled";
constexpr const char* UBSAN_FLAG_NAME = "ubsanEnabled";
constexpr const char* PERMISSION_TEMP_JIT_ALLOW = "TEMPJITALLOW";

static int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}

void AppSpawnStartMsgUtil::PresetMaxChildProcess(std::shared_ptr<AppRunningRecord> appRecord, int32_t &maxChildProcess)
{
    ProcessType processType = appRecord->GetProcessType();
    ExtensionAbilityType extensionType = appRecord->GetExtensionType();
    if (processType == ProcessType::EXTENSION && extensionType != ExtensionAbilityType::DATASHARE &&
        extensionType != ExtensionAbilityType::SERVICE) {
        maxChildProcess = 1;
    }
}

static void QueryExtensionSandBox(const std::string &moduleName, const std::string &abilityName,
    const BundleInfo &bundleInfo, AppSpawnStartMsg &startMsg, DataGroupInfoList &dataGroupInfoList, bool strictMode,
    std::shared_ptr<AAFwk::Want> want)
{
    std::vector<ExtensionAbilityInfo> extensionInfos;
    for (auto hapModuleInfo: bundleInfo.hapModuleInfos) {
        extensionInfos.insert(extensionInfos.end(), hapModuleInfo.extensionInfos.begin(),
            hapModuleInfo.extensionInfos.end());
    }
    startMsg.strictMode = strictMode;
    auto isExist = (want == nullptr) ? false : want->HasParameter(ISOLATED_SANDBOX);
    bool isolatedSandbox = false;
    if (isExist) {
        isolatedSandbox = (want == nullptr) ? false : want->GetBoolParam(ISOLATED_SANDBOX, false);
    }
    auto infoExisted = [&moduleName, &abilityName, &strictMode, &isExist, &isolatedSandbox](
                           const ExtensionAbilityInfo &info) {
        auto ret = info.moduleName == moduleName && info.name == abilityName && info.needCreateSandbox;
        if (isExist) {
            return ret && isolatedSandbox;
        }
        return ret;
    };
    auto infoIter = std::find_if(extensionInfos.begin(), extensionInfos.end(), infoExisted);
    DataGroupInfoList extensionDataGroupInfoList;
    if (infoIter != extensionInfos.end()) {
        startMsg.isolatedExtension = true;
        startMsg.extensionSandboxPath = infoIter->moduleName + "-" + infoIter->name;
        for (auto dataGroupInfo : dataGroupInfoList) {
            auto groupIdExisted = [&dataGroupInfo](const std::string &dataGroupId) {
                return dataGroupInfo.dataGroupId == dataGroupId;
            };
            if (std::find_if(infoIter->dataGroupIds.begin(), infoIter->dataGroupIds.end(), groupIdExisted) !=
                infoIter->dataGroupIds.end()) {
                extensionDataGroupInfoList.emplace_back(dataGroupInfo);
            }
        }
        startMsg.dataGroupInfoList = extensionDataGroupInfoList;
    } else {
        startMsg.dataGroupInfoList = dataGroupInfoList;
    }
}

static void SetAtomicServiceInfo(BundleType bundleType, AppSpawnStartMsg &startMsg)
{
#ifdef OHOS_ACCOUNT_ENABLED
    TAG_LOGD(AAFwkTag::APPMGR, "execute with OHOS_ACCOUNT_ENABLED on");
    if (bundleType == BundleType::ATOMIC_SERVICE) {
        TAG_LOGI(AAFwkTag::APPMGR, "app is atomic service type");
        AccountSA::OhosAccountInfo accountInfo;
        auto errCode = AccountSA::OhosAccountKits::GetInstance().GetOhosAccountInfo(accountInfo);
        if (errCode == ERR_OK) {
            TAG_LOGI(AAFwkTag::APPMGR, "getOhosAccountInfo succeed, uid %{public}s", accountInfo.uid_.c_str());
            startMsg.atomicServiceFlag = true;
            startMsg.atomicAccount = accountInfo.uid_;
        } else {
            TAG_LOGE(AAFwkTag::APPMGR, "get ohos account info:%{public}d fail", errCode);
        }
    }
#else
    TAG_LOGD(AAFwkTag::APPMGR, "execute with OHOS_ACCOUNT_ENABLED off");
#endif // OHOS_ACCOUNT_ENABLED
}

static void SetOverlayInfo(std::shared_ptr<RemoteClientManager> remoteClientManager, const std::string &bundleName,
    const int32_t userId, AppSpawnStartMsg &startMsg)
{
    if (remoteClientManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager null");
        return;
    }
    auto bundleMgrHelper = remoteClientManager->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return;
    }
    auto overlayMgrProxy = bundleMgrHelper->GetOverlayManagerProxy();
    if (overlayMgrProxy !=  nullptr) {
        std::vector<OverlayModuleInfo> overlayModuleInfo;
        TAG_LOGD(AAFwkTag::APPMGR, "Check overlay app begin.");
        HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetOverlayModuleInfoForTarget");
        auto targetRet = IN_PROCESS_CALL(overlayMgrProxy->GetOverlayModuleInfoForTarget(
            bundleName, "", overlayModuleInfo, userId));
        if (targetRet == ERR_OK && overlayModuleInfo.size() != 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "Start an overlay app process.");
            startMsg.flags = startMsg.flags | APP_OVERLAY_FLAG;
            std::string overlayInfoPaths;
            for (auto it : overlayModuleInfo) {
                overlayInfoPaths += (it.hapPath + "|");
            }
            startMsg.overlayInfo = overlayInfoPaths;
        }
    }
}

static void StartProcessVerifyPermission(const BundleInfo &bundleInfo, bool &hasAccessBundleDirReq,
                                         uint8_t &setAllowInternet, uint8_t &allowInternet,
                                         std::vector<int32_t> &gids)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    hasAccessBundleDirReq = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [] (const auto &reqPermission) {
            if (PERMISSION_ACCESS_BUNDLE_DIR == reqPermission) {
                return true;
            }
            return false;
        });

    auto token = bundleInfo.applicationInfo.accessTokenId;
    {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "AccessTokenKit::VerifyAccessToken");
        int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_INTERNET, false);
        if (result != Security::AccessToken::PERMISSION_GRANTED) {
            setAllowInternet = 1;
            allowInternet = 0;
    #ifdef APP_MGR_SERVICE_APPMS
            auto ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().SetInternetPermission(bundleInfo.uid, 0);
            TAG_LOGD(AAFwkTag::APPMGR, "SetInternetPermission, ret = %{public}d", ret);
        } else {
            auto ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().SetInternetPermission(bundleInfo.uid, 1);
            TAG_LOGD(AAFwkTag::APPMGR, "SetInternetPermission, ret = %{public}d", ret);
            gids.push_back(NETSYS_SOCKET_GROUPID);
    #endif
        }

        result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_MANAGE_VPN, false);
        if (result == Security::AccessToken::PERMISSION_GRANTED) {
            gids.push_back(BLUETOOTH_GROUPID);
        }

        if (hasAccessBundleDirReq) {
            int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token,
                PERMISSION_ACCESS_BUNDLE_DIR, false);
            if (result != Security::AccessToken::PERMISSION_GRANTED) {
                TAG_LOGE(AAFwkTag::APPMGR, "startProcess not granted");
                hasAccessBundleDirReq = false;
            }
        }
    }
}

static void SetAppEnvInfo(const BundleInfo &bundleInfo, AppSpawnStartMsg& startMsg)
{
    if (bundleInfo.applicationInfo.tsanEnabled) {
        startMsg.appEnv.emplace(TSAN_FLAG_NAME, std::to_string(1));
    } else {
        startMsg.appEnv.emplace(TSAN_FLAG_NAME, std::to_string(0));
    }

    if (bundleInfo.applicationInfo.hwasanEnabled) {
        startMsg.appEnv.emplace(HWASAN_FLAG_NAME, std::to_string(1));
    } else {
        startMsg.appEnv.emplace(HWASAN_FLAG_NAME, std::to_string(0));
    }

    if (bundleInfo.applicationInfo.ubsanEnabled) {
        startMsg.appEnv.emplace(UBSAN_FLAG_NAME, std::to_string(1));
    } else {
        startMsg.appEnv.emplace(UBSAN_FLAG_NAME, std::to_string(0));
    }

    if (!bundleInfo.applicationInfo.appEnvironments.empty()) {
        for (const auto& appEnvironment : bundleInfo.applicationInfo.appEnvironments) {
            startMsg.appEnv.emplace(appEnvironment.name, appEnvironment.value);
        }
    }
}

static void SetAppInfo(const BundleInfo &bundleInfo, AppSpawnStartMsg &startMsg)
{
    bool hasAccessBundleDirReq;
    bool tempJitAllow = false;
    uint8_t setAllowInternet = 0;
    uint8_t allowInternet = 1;
    std::vector<int32_t> gids;
    StartProcessVerifyPermission(bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    startMsg.uid = bundleInfo.uid;
    startMsg.gid = bundleInfo.gid;
    startMsg.hapFlags = bundleInfo.isPreInstallApp ? 1 : 0;
    startMsg.accessTokenId = bundleInfo.applicationInfo.accessTokenId;
    startMsg.accessTokenIdEx = bundleInfo.applicationInfo.accessTokenIdEx;
    startMsg.apl = bundleInfo.applicationInfo.appPrivilegeLevel;
    startMsg.ownerId = bundleInfo.signatureInfo.appIdentifier;
    startMsg.provisionType = bundleInfo.applicationInfo.appProvisionType;
    if (bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_MOD < API15) {
        startMsg.maxChildProcess = 0;
    } else {
        if (startMsg.maxChildProcess == 0) {
            startMsg.maxChildProcess = bundleInfo.applicationInfo.maxChildProcess;
        }
    }
    startMsg.setAllowInternet = setAllowInternet;
    startMsg.allowInternet = allowInternet;
    startMsg.gids = gids;
    startMsg.flags |= hasAccessBundleDirReq ? APP_ACCESS_BUNDLE_DIR : 0;
    tempJitAllow = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [] (const auto &reqPermission) {
            if (PERMISSION_TEMP_JIT_ALLOW == reqPermission) {
                return true;
            }
            return false;
        });
    startMsg.flags |= tempJitAllow ? START_FLAG_BASE << StartFlags::TEMP_JIT_ALLOW : 0;
    SetAppEnvInfo(bundleInfo, startMsg);
}

int32_t AppSpawnStartMsgUtil::CreateStartMsgInner(std::shared_ptr<RemoteClientManager> remoteClientManager,
    const std::string &processName, uint32_t startFlags,
    const int uid, const BundleInfo &bundleInfo, const int32_t bundleIndex, BundleType bundleType,
    AppSpawnStartMsg &startMsg, std::shared_ptr<AAFwk::Want> want, const std::string &moduleName,
    const std::string &abilityName, bool strictMode)
{
    HspList hspList;
    auto ret = bundleMgrHelper->GetBaseSharedBundleInfos(bundleInfo.name, hspList,
        AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBaseSharedBundleInfos fail: %{public}d", ret);
        return ret;
    }
    startMsg.hspList = hspList;

    auto userId = GetUserIdByUid(uid);
    DataGroupInfoList dataGroupInfoList;
    bool result = bundleMgrHelper->QueryDataGroupInfos(bundleInfo.name, userId, dataGroupInfoList);
    if (!result || dataGroupInfoList.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "the bundle has no groupInfos.");
    }
    QueryExtensionSandBox(moduleName, abilityName, bundleInfo, startMsg, dataGroupInfoList, strictMode, want);
    startMsg.bundleName = bundleInfo.name;
    startMsg.renderParam = RENDER_PARAM;
    startMsg.flags = startFlags;
    startMsg.bundleIndex = bundleIndex;
    startMsg.procName = processName;

    SetAtomicServiceInfo(bundleType, startMsg);
    SetOverlayInfo(remoteClientManager, bundleInfo.name, userId, startMsg);
    SetAppInfo(bundleInfo, startMsg);

    TAG_LOGI(AAFwkTag::APPMGR, "apl: %{public}s, bundleName: %{public}s, startFlags: %{public}d",
        startMsg.apl.c_str(), bundleInfo.name.c_str(), startFlags);

    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
