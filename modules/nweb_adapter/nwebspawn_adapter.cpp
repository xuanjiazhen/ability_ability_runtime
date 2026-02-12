/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <cerrno>
#include <ctime>
#include <dlfcn.h>
#include <map>
#include <mutex>
#include <string>

#ifdef __MUSL__
#include <cerrno>
#include <dlfcn_ext.h>
#include <sys/mman.h>
#endif

#include "appspawn_service.h"
#include "appspawn_hook.h"
#include "appspawn_manager.h"

#ifdef WITH_SECCOMP
#include "seccomp_policy.h"
#endif
#ifndef OHOS_LITE
#include "ace_forward_compatibility.h"
#endif
#include "arkweb_utils.h"
#include "arkweb_preload_common.h"
#include "securec.h"

struct RenderIpcFds {
    int32_t ipcFd;
    int32_t sharedFd;
    int32_t crashFd;
};

namespace {
const std::string ARK_WEB_ENGINE_LIB_NAME = "libarkweb_engine.so";
const std::string ARK_WEB_RENDER_LIB_NAME = "libarkweb_render.so";
const std::string RENDER_IPC_FD_SUFFIX = "#--ipc-fd=";
const std::string RENDER_SHARED_FD_SUFFIX = "#--shared-fd=";
const std::string RENDER_CRASH_FD_SUFFIX = "#--crash-fd=";
static RenderIpcFds g_renderIpcFds = {-1, -1, -1};
}  // namespace

static bool SetSeccompPolicyForRenderer(void *nwebRenderHandle)
{
#ifdef WITH_SECCOMP
    if (IsEnableSeccomp()) {
        using SeccompFuncType = bool (*)(void);
        SeccompFuncType funcSetRendererSeccompPolicy =
                reinterpret_cast<SeccompFuncType>(dlsym(nwebRenderHandle, "SetRendererSeccompPolicy"));
        if (funcSetRendererSeccompPolicy != nullptr && funcSetRendererSeccompPolicy()) {
            return true;
        }
        APPSPAWN_LOGE("SetRendererSeccompPolicy dlsym errno: %{public}d", errno);
        return false;
    }
#endif
    return true;
}

static OHOS::ArkWeb::ArkWebEngineVersion UpdateAppWebEngineVersion(std::string& renderCmd)
{
    size_t posLeft = renderCmd.rfind(APP_ENGINE_VERSION_PREFIX);
    if (posLeft == std::string::npos) {
        APPSPAWN_LOGE("not found app engine type arg");
        return OHOS::ArkWeb::ArkWebEngineVersion::SYSTEM_DEFAULT;
    }
    size_t posRight = posLeft + strlen(APP_ENGINE_VERSION_PREFIX);
    size_t posEnd = renderCmd.find('#', posRight);
    std::string value = (posEnd == std::string::npos) ?
                            renderCmd.substr(posRight) :
                            renderCmd.substr(posRight, posEnd - posRight);

    char* end;
    long v = std::strtol(value.c_str(), &end, 10);
    if (*end != '\0' || v > INT_MAX || v < 0) {
        APPSPAWN_LOGE("invalid value: %{public}s", value.c_str());
        return OHOS::ArkWeb::ArkWebEngineVersion::SYSTEM_DEFAULT;
    }
    auto version = static_cast<OHOS::ArkWeb::ArkWebEngineVersion>(v);
    OHOS::ArkWeb::SetActiveWebEngineVersionInner(version);

    return version;
}

APPSPAWN_STATIC void EarseAppWebEngineVersion(std::string& renderCmd)
{
    size_t posLeft = renderCmd.rfind(APP_ENGINE_VERSION_PREFIX);
    if (posLeft == std::string::npos) {
        APPSPAWN_LOGE("not found app engine type arg");
        return OHOS::ArkWeb::ArkWebEngineVersion::SYSTEM_DEFAULT;
    }
    size_t posRight = posLeft + strlen(APP_ENGINE_VERSION_PREFIX);
    size_t posEnd = renderCmd.find('#', posRight);
    
    // remove arg APP_ENGINE_VERSION_PREFIX
    size_t eraseLength = (posEnd == std::string::npos) ?
                            renderCmd.length() - posLeft :
                            posEnd - posLeft;
    renderCmd.erase(posLeft, eraseLength);
}

APPSPAWN_STATIC int AddRenderIpcFdsToCmd(std::string& renderCmd)
{
    renderStr += RENDER_IPC_FD_SUFFIX + std::to_string(g_renderIpcFds.ipcFd);
    renderStr += RENDER_SHARED_FD_SUFFIX + std::to_string(g_renderIpcFds.sharedFd);
    renderStr += RENDER_CRASH_FD_SUFFIX + std::to_string(g_renderIpcFds.crashFd);
    if (renderStr.length() > APP_RENDER_CMD_MAX_LEN) {
        return APPSPAWN_MSG_TOO_LONG;
    }
    return APPSPAWN_OK;
}

APPSPAWN_STATIC int RunChildProcessor(AppSpawnContent *content, AppSpawnClient *client)
{
    uint32_t len = 0;
    char *renderCmd = reinterpret_cast<char *>(GetAppPropertyExt(
        reinterpret_cast<AppSpawningCtx *>(client), MSG_EXT_NAME_RENDER_CMD, &len));
    if (renderCmd == nullptr) {
        return -1;
    }

    std::string renderStr(renderCmd);
    EarseAppWebEngineVersion(renderStr);
    OHOS::ArkWeb::UpdateAppInfoFromCmdline(renderStr);
    if (AddRenderIpcFdsToCmd(renderStr) != APPSPAWN_OK) {
        APPSPAWN_LOGE("renderCmd too long: %{public}zu > %{public}d",
                      renderStr.length(), APP_RENDER_CMD_MAX_LEN);
        return APPSPAWN_MSG_TOO_LONG;
    }

    void *webEngineHandle = nullptr;
    void *nwebRenderHandle = nullptr;

    const std::string libNsName = OHOS::ArkWeb::GetArkwebNameSpace();
    const std::string libPath = OHOS::ArkWeb::GetArkwebLibPath();
    const std::string engineLibName = ARK_WEB_ENGINE_LIB_NAME;
    const std::string renderLibName = ARK_WEB_RENDER_LIB_NAME;

#ifdef __MUSL__
    Dl_namespace dlns;
    Dl_namespace ndkns;
    dlns_init(&dlns, libNsName.c_str());
    dlns_create(&dlns, libPath.c_str());
    dlns_get("ndk", &ndkns);
    dlns_inherit(&dlns, &ndkns, "allow_all_shared_libs");
    // preload libweb_engine
    webEngineHandle = OHOS::ArkWeb::LoadWithRelroFile(engineLibName, &dlns);
    if (webEngineHandle) {
        APPSPAWN_LOGI("dlopen_ns_ext success!!!");
    } else {
        APPSPAWN_LOGW("dlopen_ns_ext failed, fallback to dlopen_ns");
        webEngineHandle = dlopen_ns(&dlns, engineLibName.c_str(), RTLD_NOW | RTLD_GLOBAL);
    }
    // load libnweb_render
    nwebRenderHandle = dlopen_ns(&dlns, renderLibName.c_str(), RTLD_NOW | RTLD_GLOBAL);
#else
    // preload libweb_engine
    const std::string engineLibPath = libPath + "/" + engineLibName;
    webEngineHandle = dlopen(engineLibPath.c_str(), RTLD_NOW | RTLD_GLOBAL);

    // load libnweb_render
    const std::string renderLibPath = libPath + "/" + renderLibName;
    nwebRenderHandle = dlopen(renderLibPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
    if (webEngineHandle == nullptr) {
        APPSPAWN_LOGE("Fail to dlopen libweb_engine.so, errno: %{public}d", errno);
    }
    if (nwebRenderHandle == nullptr) {
        APPSPAWN_LOGE("Fail to dlopen libnweb_render.so, errno: %{public}d", errno);
        return -1;
    }

    char *processTypeChar = reinterpret_cast<char *>(GetAppPropertyExt(
        reinterpret_cast<AppSpawningCtx *>(client), MSG_EXT_NAME_PROCESS_TYPE, &len));
    std::string processType = (processTypeChar != nullptr) ? std::string(processTypeChar) : "";
    if (processType == "render" && !SetSeccompPolicyForRenderer(nwebRenderHandle)) {
        return -1;
    }
    using FuncType = void (*)(const char *cmd);

    FuncType funcNWebRenderMain = reinterpret_cast<FuncType>(dlsym(nwebRenderHandle, "NWebRenderMain"));
    if (funcNWebRenderMain == nullptr) {
        APPSPAWN_LOGE("webviewspawn dlsym errno: %{public}d", errno);
        return -1;
    }
    AppSpawnEnvClear(content, client);
    APPSPAWN_LOGI("RunChildProcessorNweb %{public}s", renderStr.c_str());
    funcNWebRenderMain(renderStr);
    return 0;
}

APPSPAWN_STATIC int ParseRenderIpcFds(AppSpawnMsgNode *message, const int *fds, RenderIpcFds &origFds)
{
    origFds.ipcFd = -1;
    origFds.sharedFd = -1;
    origFds.crashFd = -1;

    int findFdIndex = 0;
    for (uint32_t index = TLV_MAX; index < (TLV_MAX + message->tlvCount); index++) {
        if (message->tlvOffset[index] == INVALID_OFFSET) {
            return APPSPAWN_ARG_INVALID;
        }
        uint8_t *data = message->buffer + message->tlvOffset[index];
        if (((AppSpawnTlv *)data)->tlvType != TLV_MAX) {
            continue;
        }
        AppSpawnTlvExt *tlv = (AppSpawnTlvExt *)data;
        if (strcmp(tlv->tlvName, MSG_EXT_NAME_APP_FD) != 0) {
            continue;
        }
        std::string key((char *)data + sizeof(AppSpawnTlvExt));
        if (fds[findFdIndex] <= 0) {
            return APPSPAWN_ARG_INVALID;
        }

        if (key == "ipc-fd") {
            origFds.ipcFd = fds[findFdIndex];
        } else if (key == "shared-fd") {
            origFds.sharedFd = fds[findFdIndex];
        } else if (key == "crash-fd") {
            origFds.crashFd = fds[findFdIndex];
        }
        findFdIndex++;

        if (origFds.ipcFd > 0 && origFds.sharedFd > 0 && origFds.crashFd > 0) {
            break;
        }
    }

    if (origFds.ipcFd < 0 || origFds.sharedFd < 0 || origFds.crashFd < 0) {
        APPSPAWN_LOGE("Received render ipc fds invalid, ipcFd=%{public}d, sharedFd=%{public}d, crashFd=%{public}d",
                      origFds.ipcFd, origFds.sharedFd, origFds.crashFd);
        return APPSPAWN_ARG_INVALID;
    }
    return APPSPAWN_OK;
}

APPSPAWN_STATIC int DupRenderIpcFds(const RenderIpcFds &origFds)
{
    g_renderIpcFds.ipcFd = dup(origFds.ipcFd);
    if (g_renderIpcFds.ipcFd <= 0) {
        APPSPAWN_LOGE("Dup ipcFd failed, errno=%{public}d", errno);
        return APPSPAWN_ARG_INVALID;
    }

    g_renderIpcFds.sharedFd = dup(origFds.sharedFd);
    if (g_renderIpcFds.sharedFd <= 0) {
        APPSPAWN_LOGE("Dup sharedFd failed, errno=%{public}d", errno);
        close(g_renderIpcFds.ipcFd);
        g_renderIpcFds.ipcFd = -1;
        return APPSPAWN_ARG_INVALID;
    }

    g_renderIpcFds.crashFd = dup(origFds.crashFd);
    if (g_renderIpcFds.crashFd <= 0) {
        APPSPAWN_LOGE("Dup crashFd failed, errno=%{public}d", errno);
        close(g_renderIpcFds.ipcFd);
        close(g_renderIpcFds.sharedFd);
        g_renderIpcFds.ipcFd = -1;
        g_renderIpcFds.sharedFd = -1;
        return APPSPAWN_ARG_INVALID;
    }

    APPSPAWN_LOGV("Dup render ipc fds success. ipcFd=%{public}d, sharedFd=%{public}d, crashFd=%{public}d",
                  g_renderIpcFds.ipcFd, g_renderIpcFds.sharedFd, g_renderIpcFds.crashFd);

    return APPSPAWN_OK;
}

APPSPAWN_STATIC int DupNwebRenderFdsBeforeRunHook(AppSpawnMgr *content, AppSpawningCtx *property)
{
    if (!IsNWebSpawnMode(content)) {
        return APPSPAWN_ARG_INVALID;
    }

    uint32_t len = 0;
    char *renderCmd = reinterpret_cast<char *>(GetAppPropertyExt(property, MSG_EXT_NAME_RENDER_CMD, &len));
    if (renderCmd == nullptr || len == 0) {
        return APPSPAWN_ARG_INVALID;
    }

    std::string renderStr(renderCmd);
    auto version = UpdateAppWebEngineVersion(renderStr);
    if (version == OHOS::ArkWeb::ArkWebEngineVersion::M114) {
        APPSPAWN_LOGV("M114 version, skip fd processing");
        return APPSPAWN_OK;
    }

    APPSPAWN_CHECK_ONLY_EXPER(property != NULL, return APPSPAWN_ARG_INVALID);
    AppSpawnMsgNode *message = property->message;
    APPSPAWN_CHECK_ONLY_EXPER(message != NULL && message->buffer != NULL && message->connection != NULL,
                              return APPSPAWN_ARG_INVALID);
    APPSPAWN_CHECK_ONLY_EXPER(message->tlvOffset != NULL, return APPSPAWN_TLV_NONE);

    AppSpawnMsgReceiverCtx recvCtx = message->connection->receiverCtx;
    APPSPAWN_CHECK_LOGV(recvCtx.fds != NULL && recvCtx.fdCount > 0, return 0,
                        "no need set fd info %{public}d, %{public}d", recvCtx.fds != NULL, recvCtx.fdCount);

    RenderIpcFds origFds {};
    int ret = ParseRenderIpcFds(message, recvCtx.fds, origFds);
    if (ret != APPSPAWN_OK) {
        return ret;
    }

    return DupRenderIpcFds(origFds);
}

APPSPAWN_STATIC int PreLoadNwebSpawn(AppSpawnMgr *content)
{
    APPSPAWN_LOGI("PreLoadNwebSpawn %{public}d", IsNWebSpawnMode(content));
    if (!IsNWebSpawnMode(content)) {
        return 0;
    }
#ifdef __MUSL__
    if (OHOS::ArkWeb::ReserveAddressSpace()) {
        OHOS::ArkWeb::CreateRelroFileInSubProc();
    }
#endif
    // register
    RegChildLooper(&content->content, RunChildProcessor);

    OHOS::ArkWeb::PreloadArkWebLibForRender();

#ifndef OHOS_LITE
    OHOS::Ace::AceForwardCompatibility::ReclaimFileCache(getpid());
#endif
    return 0;
}

MODULE_CONSTRUCTOR(void)
{
    APPSPAWN_LOGI("Load nweb module ...");
    AddPreloadHook(HOOK_PRIO_HIGHEST, PreLoadNwebSpawn);
    // Execute dup fd in child process (render) before notifying parent
    // STAGE_CHILD_PRE_RELY runs before NotifyResToParent
    AddAppSpawnHook(STAGE_CHILD_PRE_RELY, HOOK_PRIO_HIGHEST, DupNwebRenderFdsBeforeRunHook);
}
