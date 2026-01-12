/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS::AAFwk {
class IsMockSaCall {
public:
    static void IsMockSaCallWithPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.ACCESS_DLP_FILE",
            perms[1] = "ohos.permission.CLEAN_APPLICATION_DATA",
            perms[2] = "ohos.permission.CLEAN_BACKGROUND_PROCESSES",
            perms[3] = "ohos.permission.GET_RUNNING_INFO",
            perms[4] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS",
            perms[5] = "ohos.permission.MANAGE_MISSIONS",
            perms[6] = "ohos.permission.RUNNING_STATE_OBSERVER",
            perms[7] = "ohos.permission.SET_ABILITY_CONTROLLER",
            perms[8] = "ohos.permission.UPDATE_CONFIGURATION",
            perms[9] = "ohos.permission.INSTALL_BUNDLE",
            perms[10] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED",
            perms[11] = "ohos.permission.START_INVISIBLE_ABILITY",
            perms[12] = "ohos.permission.START_ABILITIES_FROM_BACKGROUND",
            perms[13] = "ohos.permission.START_ABILIIES_FROM_BACKGROUND",
            perms[14] = "ohos.permission.ABILITY_BACKGROUND_COMMUNICATION",
            perms[15] = "ohos.permission.MANAGER_ABILITY_FROM_GATEWAY",
            perms[16] = "ohos.permission.PROXY_AUTHORIZATION_URI",
            perms[17] = "ohos.permission.EXEMPT_AS_CALLER",
            perms[18] = "ohos.permission.EXEMPT_AS_TARGET",
            perms[19] = "ohos.permission.PREPARE_APP_TERMINATE",
            perms[20] = "ohos.permission.START_ABILITY_WITH_ANIMATION",
            perms[21] = "ohos.permission.MANAGE_APP_BOOT_INTERNAL",
            perms[22] = "ohos.permission.CONNECT_UI_EXTENSION_ABILITY",
            perms[23] = "ohos.permission.START_RECENT_ABILITY"
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "distributedsched";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockCheckObserverCallerPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.RUNNING_STATE_OBSERVER",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "memmgrservice";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockProcessCachePermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.SET_PROCESS_CACHE_STATE",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "distributedsched";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockSpecificSystemAbilityAccessPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.SET_PROCESS_CACHE_STATE",
            perms[1] = "ohos.permission.GET_RUNNING_INFO",
            perms[2] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS",
            perms[3] = "ohos.permission.UPDATE_CONFIGURATION",
            perms[4] = "ohos.permission.ACCESS_DEVICE_COLLABORATION_SERVICE",
            perms[5] = "ohos.permission.INPUT_MONITORING",
            perms[6] = "ohos.permission.PERMISSION_USED_STATS",
            perms[7] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER",
            perms[8] = "ohos.permission.DISTRIBUIED_DATASYNC",
            perms[9] = "ohos.permission.MANAGE_AUDIO_CONFIG",
            perms[10] = "ohos.permission.WRITE_CALL_LOG",
            perms[11] = "ohos.permission.READ_CALL_LOG",
            perms[12] = "ohos.permission.READ_CONTACTS",
            perms[13] = "ohos.permission.READ_DFX_SYSEVENT",
            perms[14] = "ohos.permission.GRANT_SENSITIVE_PERMISSIONS",
            perms[15] = "ohos.permission.REVOKE_SENSITIVE_PERMISSIONS",
            perms[16] = "ohos.permission.MANAGE_SETTINGS",
            perms[17] = "ohos.permission.MANAGE_SECURE_BACKGROUND",
            perms[18] = "ohos.permission.START_ABILITYS_FROM_BACKGROUND",
            perms[19] = "ohos.permission.ACCESS_SERVICE_DM",
            perms[20] = "ohos.permission.ACCESS_SERVICE_DP",
            perms[21] = "ohos.permission.STORAGE_MANAGER",
            perms[22] = "ohos.permission.PROXY_AUTHORIZATION_URI",
            perms[23] = "ohos.permission.ABILITY_BACKGROUND_COMMUNICATION",
            perms[24] = "ohos.permission.USE_USER_IDM",
            perms[25] = "ohos.permission.MANAGE_LOCAL_ACCOUNTS",
            perms[26] = "ohos.permission.LISTEN_BUNDLE_CHANGE",
            perms[27] = "ohos.permission.GET_TELEPHONY_STATE",
            perms[28] = "ohos.permission.SEND_MESSAGES",
            perms[29] = "ohos.permission.CONNECT_CELLULAR_CALL_SERVICE",
            perms[30] = "ohos.permission.SET_TELEPHONY_STATE",
            perms[31] = "ohos.permission.VIBRATE",
            perms[32] = "ohos.permission.SYSTEM_LIGHT_CONTROL",
            perms[33] = "ohos.permission.MANAGE_HAP_TOKENID",
            perms[34] = "ohos.permission.UPDATE_CONIGURATION",
            perms[35] = "ohos.permission.WRITE_WHOLE_CALENDAR",
            perms[36] = "ohos.permission.PEPORT_RESOURCE_SCHEDULE_EVENT",
            perms[37] = "ohos.permission.START_INVISIBLE_ABILITY",
            perms[38] = "ohos.permission.GET_BUNDLE_INFO",
            perms[39] = "ohos.permission.GET_SUSPEND_STATE",
            perms[40] = "ohos.permission.PUBLISH_SYSTEM_COMMON_EVENT",
            perms[41] = "ohos.permission.PUBLISH_DISPLAY_ROTATION_EVENT",
            perms[42] = "ohos.permission.PUBLISH_CAST_PLUGGED_EVENT",
            perms[43] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED",
            perms[44] = "ohos.permission.CLEAM_APPLICATION_DATA",
            perms[45] = "ohos.permission.REMOVE_CACHE_FILES",
            perms[46] = "ohos.permission.INSTALL_SANDBOX_BUNDLE",
            perms[47] = "ohos.permission.USE_BLUETOOTH",
            perms[48] = "ohos.permission.GET_SENSITIVE_PERMISSIONS",
            perms[49] = "ohos.permission.CONNECTIVITY_INTERNAL",
            perms[50] = "ohos.permission.GET_RUNNING_INFO",
            perms[51] = "ohos.permission.INTERCACT_ACROSS_LOCAL_ACCOUNTS",
            perms[52] = "ohos.permission.ACCESS_BLUETOOTH",
            perms[53] = "ohos.permission.MANAGE_BLUETOOTH",
            perms[54] = "ohos.permission.ACCESS_NEARLINK",
            perms[55] = "ohos.permission.MANAGE_NEARLINK",
            perms[56] = "ohos.permission.RUNNING_STATE_OBSERVER",
            perms[57] = "ohos.permission.GET_INSTALLED_BUNDLE_LIST",
            perms[58] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS_EXTENSION",
            perms[59] = "ohos.permission.MANAGE_USER_ACCOUNT_INFO",
            perms[60] = "ohos.permission.GET_NETWORK_INFO",
            perms[61] = "ohos.permission.VERIFY_ACTIVATION_LOCK",
            perms[62] = "ohos.permission.CAPTURE_SCREEN",
            perms[63] = "ohos.permission.GRANT_URI_PERMISSION_PRIVILEGED",
            perms[64] = "ohos.permission.START_SYSTEM_DIALOG",
            perms[65] = "ohos.permission.GET_BUNDLE_RESOURCES",
            perms[66] = "ohos.permission.REQUIRE_FORM",
            perms[67] = "ohos.permission.MODIFY_AUDIO_SETTINGS",
            perms[68] = "ohos.permission.ACCESS_SPAMSHIELD_SERVICE",
            perms[69] = "ohos.permission.APPROXIMATELY_LOCATON",
            perms[70] = "ohos.permission.LOCATION",
            perms[71] = "ohos.permission.LOCATION_IN_BACKGROUND",
            perms[72] = "ohos.permission.BUNDLE_ACTIVE_INFO",
            perms[73] = "ohos.permission.GET_SUPER_PRIVACY",
            perms[74] = "ohos.permission.FILE_ACCESS_PERSIST",
            perms[75] = "ohos.permission.SET_SANDBOX_POLICY",
            perms[76] = "ohos.permission.CHECK_SANDBOX_POLICY",
            perms[77] = "ohos.permission.SET_SUPER_PRIVACY",
            perms[78] = "ohos.permission.ACCESS_USER_AUTH_INTERNAL",
            perms[79] = "ohos.permission.MANAGE_MISSIONS",
            perms[80] = "ohos.permission.CONNECT_FORM_EXTENSION",
            perms[81] = "ohos.permission.CONNECT_STATIC_SUBSCRIBER_EXTENSION",
            perms[82] = "ohos.permission.KILL_PROCESS_DEPENDED_ON_ARKWEB",
            perms[83] = "ohos.permission.ACCESS_LOWPOWER_MANAGER",
            perms[84] = "ohos.permission.RECEIVER_STARTUP_COMPLETED",
            perms[85] = "ohos.permission.hsdr.HSDR_ACCESS",
            perms[86] = "ohos.permission.START_RESTONE_NOTIFICATION",
            perms[87] = "ohos.permission.ATTEST_KEY",
            perms[88] = "ohos.permission.READ_WHOLE_CALENDAR",
            perms[89] = "ohos.permission.MONITOR_DEVICE_NETWORK_STATE",
            perms[90] = "ohos.permission.NOTIFICATION_CONTROLLER",
            perms[91] = "ohos.permission.ACCESS_SCREEN_LOCK",
            perms[92] = "ohos.permission.hsdr.REQUEST_HSDR",
            perms[93] = "ohos.permission.ACCESS_CUSTIOM_RINGTONE",
            perms[94] = "ohos.permission.ACCESS_SEARCH_SERVICE",
            perms[95] = "ohos.permission.RECEIVE_BMS_BROKER_MESSAGES",
            perms[96] = "ohos.permission.RECEIVE_UPDATA_MESSAGE",
            perms[97] = "ohos.permission.WRITE_RINGTONE",
            perms[98] = "ohos.permission.ACCESS_SCR"
            perms[99] = 
            perms[100] = 
            perms[101] = 
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "foundation";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockKillAppProcessesPermission()
    {
        int permission = 0;
        uint64_t tokenId;
        const char** perms = new const char* [1];
        perms[permission] = "ohos.permission.KILL_APP_PROCESSES";
        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 1,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };

        infoInstance.processName = "accesstoken_service";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        delete[] perms;
    }
};
}  // namespace OHOS::AAFwk
#endif // UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H