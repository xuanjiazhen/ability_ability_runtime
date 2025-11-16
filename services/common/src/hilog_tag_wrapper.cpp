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

#include "hilog_tag_wrapper.h"

#include <unordered_map>

namespace OHOS {
namespace AAFwk {
namespace {
constexpr uint32_t BASE_DEFAULT = 0;
constexpr uint32_t BASE_APPDFR = 1;
constexpr uint32_t BASE_JSENV = 2;
constexpr uint32_t BASE_DELEGATOR = 3;
constexpr uint32_t BASE_EXT = 4;
constexpr uint32_t BASE_WANTAGENT = 5;
constexpr uint32_t BASE_LOCAL_CALL = 6;

std::unordered_map<std::string, std::string> abbrFileNameMap {
    { "ability_connect_manager", "ABCM" },
    { "ability_manager_service", "ABMS" },
    { "ability_record", "ABR" },
    { "ability_connect_callback_stub", "ACCS" },
    { "ability_manager_client", "AMC" },
    { "app_mgr_service_inner", "AMSI" },
    { "app_running_manager", "ARM" },
    { "app_running_record", "ARR" },
    { "app_spawn_client", "ASC" },
    { "app_state_observer_manager", "ASOM" },
    { "cache_process_manager", "CPM" },
    { "connection_record", "CONN" },
    { "connect_server_manager", "CSM" },
    { "dump_runtime_helper", "DRH" },
    { "ecological_rule_interceptor", "ERI" },
    { "extension_record_manager", "ERM" },
    { "js_ability_stage", "JAS" },
    { "js_runtime_common", "JRC" },
    { "js_service_extension", "JSE" },
    { "js_environment", "JSENV" },
    { "js_service_extension_context", "JSEC" },
    { "js_ui_ability", "JUA" },
    { "main_thread", "MTHRD" },
    { "ohos_application", "OH_APP" },
    { "preload_uiext_state_observer", "PUSO" },
    { "service_extension_context", "SEC" },
    { "ui_ability_lifecycle_manager", "UALM" },
    { "window_pid_visibility_changed_listener", "WND_CHG_LSNR" }
};
}

uint32_t GetOffset(AAFwkLogTag tag, AAFwkLogTag base)
{
    return static_cast<uint32_t>(tag) - static_cast<uint32_t>(base);
}

const char* GetDomainName0(AAFwkLogTag tag)
{
    static const char* tagNames[] = { "AAFwk", "Ability", "Test", "AATool", "Simulator" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DEFAULT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

const char* GetDomainName1(AAFwkLogTag tag)
{
    static const char* tagNames[] = { "AppDfr", "AppMS", "DbObsMgr", "Dialog", "Quickfix",
        "UriPermMgr", "BMSHelper", "AppKit" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::APPDFR);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

const char* GetDomainName2(AAFwkLogTag tag)
{
    static const char* tagNames[] = { "JsEnv", "JsRuntime", "FA", "Intent", "JsNapi",
        "CjRuntime", "EtsRuntime", "Ani"};
    uint32_t offset = GetOffset(tag, AAFwkLogTag::JSENV);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

const char* GetDomainName3(AAFwkLogTag tag)
{
    static const char* tagNames[] = { "Delegator", "Context", "UIAbility", "Want", "Mission",
        "Connection", "AMS", "EcologicalRule", "DataAbility" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DELEGATOR);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

const char* GetDomainName4(AAFwkLogTag tag)
{
    static const char* tagNames[] = { "Ext", "AutoFillExt", "ServiceExt", "FormExt", "ShareExt",
        "UIExt", "ActionExt", "EmbeddedExt", "UIServiceExt" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::EXT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

const char* GetDomainName5(AAFwkLogTag tag)
{
    static const char* tagNames[] = { "WantAgent", "AutoFillMgr", "ExtMgr", "ServiceRouter",
        "AutoStartup", "Startup", "Recovery", "ProcessMgr", "Continuation",
        "Distributed", "FreeInstall", "KeepAlive", "QueryERMS" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::WANTAGENT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

const char* GetDomainName6(AAFwkLogTag tag)
{
    static const char* tagNames[] = { "LocalCall", "SaInterceptor", "AppServiceExt", "VerticalPanel" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::LOCAL_CALL);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

const char* GetTagInfoFromDomainId(AAFwkLogTag tag)
{
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DEFAULT);
    uint32_t base = offset >> 4;
    switch (base) {
        case BASE_DEFAULT: return GetDomainName0(tag);
        case BASE_APPDFR: return GetDomainName1(tag);
        case BASE_JSENV: return GetDomainName2(tag);
        case BASE_DELEGATOR: return GetDomainName3(tag);
        case BASE_EXT: return GetDomainName4(tag);
        case BASE_WANTAGENT: return GetDomainName5(tag);
        case BASE_LOCAL_CALL: return GetDomainName6(tag);
        default: return "UN";
    }
}

std::string ConvertFileName(const std::string& fileName)
{
    std::string fileBaseName = fileName;
    auto pos = fileName.find_last_of(".");
    if (pos == std::string::npos) {
        return fileBaseName;
    }
    fileBaseName = fileName.substr(0, pos);
    if (abbrFileNameMap.find(fileBaseName) != abbrFileNameMap.end()) {
        return abbrFileNameMap[fileBaseName];
    }
    return fileBaseName;
}
}  // namespace AAFwk
}  // namespace OHOS