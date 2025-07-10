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

#include "cj_environment.h"

#include <string>
#include <sstream>
#include <mutex>
#include <charconv>
#include "cj_hilog.h"
#include "cj_invoker.h"
#ifdef __OHOS__
#include <dlfcn.h>
#endif
#include "dynamic_loader.h"
#ifdef WITH_EVENT_HANDLER
#include "event_handler.h"
#endif

#ifdef APP_USE_ARM64
#define APP_LIB_NAME "arm64"
#elif defined(APP_USE_ARM)
#define APP_LIB_NAME "arm"
#elif defined(APP_USE_X86_64)
#define APP_LIB_NAME "x86_64"
#elif defined(NAPI_TARGET_ARM64)
#define APP_LIB_NAME "arm64"
#else
#error unsupported platform
#endif

namespace {
const std::string SANDBOX_LIB_PATH = "/data/storage/el1/bundle/libs/" APP_LIB_NAME;
const std::string CJ_RT_PATH = SANDBOX_LIB_PATH + "/runtime";
const std::string CJ_LIB_PATH = SANDBOX_LIB_PATH + "/ohos";
const std::string CJ_SYSLIB_PATH = "/system/lib64:/system/lib64/platformsdk";
const std::string CJ_CHIPSDK_PATH = "/system/lib64/chipset-pub-sdk:/system/lib64/chipset-sdk";
const std::string CJ_SDK_PATH = "/system/lib64/platformsdk/cjsdk";
const std::string CJ_RUNTIME_PATH = "/system/lib64/platformsdk/cjsdk/runtime";
const std::string CJ_ASAN_PATH = SANDBOX_LIB_PATH + "/asan";
} // namespace

namespace OHOS {
namespace {
const char DEBUGGER_LIBNAME[] = "libcj_debugger.z.so";
const char DEBUGGER_SYMBOL_NAME[] = "StartDebuggerServer";
const char INIT_CJRUNTIME_SYMBOL_NAME[] = "InitCJRuntime";
const char INIT_UISCHEDULER_SYMBOL_NAME[] = "InitUIScheduler";
const char RUN_UISCHEDULER_SYMBOL_NAME[] = "RunUIScheduler";
const char FINI_CJRUNTIME_SYMBOL_NAME[] = "FiniCJRuntime";
const char INIT_CJLIBRARY_SYMBOL_NAME[] = "InitCJLibrary";
const char REGISTER_EVENTHANDLER_CALLBACKS_NAME[] = "RegisterEventHandlerCallbacks";
const char REGISTER_ARKVM_SYMBOL_NAME[] = "RegisterArkVMInRuntime";
const char REGISTER_STACKINFO_CALLBACKS_NAME[] = "RegisterStackInfoCallbacks";
const char DUMP_HEAP_SNAPSHOT_NAME[] = "CJ_MRT_DumpHeapSnapshot";
const char FORCE_FULL_GC_NAME[] = "CJ_MRT_ForceFullGC";

using InitCJRuntimeType = int(*)(const struct RuntimeParam*);;
using InitUISchedulerType = void*(*)();
using RunUISchedulerType = int(*)(unsigned long long);
using FiniCJRuntimeType = int(*)();
using InitCJLibraryType = int(*)(const char*);
using RegisterEventHandlerType = void(*)(PostTaskType, HasHigherPriorityType);
using RegisterArkVMType = void(*)(unsigned long long);
using RegisterStackInfoType = void(*)(UpdateStackInfoFuncType);
using DumpHeapSnapshotType = void(*)(int);
using ForceFullGCType = void(*)();

#ifdef __OHOS__
const char REGISTER_UNCAUGHT_EXCEPTION_NAME[] = "RegisterUncaughtExceptionHandler";
using RegisterUncaughtExceptionType = void (*)(const CJUncaughtExceptionInfo& handle);
#endif

#ifdef WITH_EVENT_HANDLER
static std::shared_ptr<AppExecFwk::EventHandler> g_handler = nullptr;
#endif

bool LoadSymbolInitCJRuntime(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_CJRUNTIME_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_CJRUNTIME_SYMBOL_NAME);
        return false;
    }
    apis.InitCJRuntime = reinterpret_cast<InitCJRuntimeType>(symbol);
    return true;
}

bool LoadSymbolInitUIScheduler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_UISCHEDULER_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_UISCHEDULER_SYMBOL_NAME);
        return false;
    }
    apis.InitUIScheduler = reinterpret_cast<InitUISchedulerType>(symbol);
    return true;
}

bool LoadSymbolRunUIScheduler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, RUN_UISCHEDULER_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", RUN_UISCHEDULER_SYMBOL_NAME);
        return false;
    }
    apis.RunUIScheduler = reinterpret_cast<RunUISchedulerType>(symbol);
    return true;
}

bool LoadSymbolFiniCJRuntime(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, FINI_CJRUNTIME_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", FINI_CJRUNTIME_SYMBOL_NAME);
        return false;
    }
    apis.FiniCJRuntime = reinterpret_cast<FiniCJRuntimeType>(symbol);
    return true;
}

bool LoadSymbolInitCJLibrary(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_CJLIBRARY_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_CJLIBRARY_SYMBOL_NAME);
        return false;
    }
    apis.InitCJLibrary = reinterpret_cast<InitCJLibraryType>(symbol);
    return true;
}

bool LoadSymbolRegisterEventHandlerCallbacks(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_EVENTHANDLER_CALLBACKS_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_EVENTHANDLER_CALLBACKS_NAME);
        return false;
    }
    apis.RegisterEventHandlerCallbacks = reinterpret_cast<RegisterEventHandlerType>(symbol);
    return true;
}

bool LoadSymbolRegisterStackInfoCallbacks(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_STACKINFO_CALLBACKS_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_STACKINFO_CALLBACKS_NAME);
        // return true for compatible.
        apis.RegisterStackInfoCallbacks = nullptr;
        return true;
    }
    apis.RegisterStackInfoCallbacks = reinterpret_cast<RegisterStackInfoType>(symbol);
    return true;
}

bool LoadSymbolRegisterArkVM(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_ARKVM_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_ARKVM_SYMBOL_NAME);
        // return true for compatible.
        apis.RegisterArkVMInRuntime = nullptr;
        return true;
    }
    apis.RegisterArkVMInRuntime = reinterpret_cast<RegisterArkVMType>(symbol);
    return true;
}

#ifdef __OHOS__
bool LoadSymbolRegisterCJUncaughtExceptionHandler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_UNCAUGHT_EXCEPTION_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_UNCAUGHT_EXCEPTION_NAME);
        return false;
    }
    apis.RegisterCJUncaughtExceptionHandler = reinterpret_cast<RegisterUncaughtExceptionType>(symbol);
    return true;
}
#endif

bool LoadSymbolDumpHeapSnapshot(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, DUMP_HEAP_SNAPSHOT_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", DUMP_HEAP_SNAPSHOT_NAME);
        // return true for compatible.
        apis.DumpHeapSnapshot = nullptr;
        return true;
    }
    apis.DumpHeapSnapshot = reinterpret_cast<DumpHeapSnapshotType>(symbol);
    return true;
}

bool LoadSymbolForceFullGC(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, FORCE_FULL_GC_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", FORCE_FULL_GC_NAME);
        // return true for compatible.
        apis.ForceFullGC = nullptr;
        return true;
    }
    apis.ForceFullGC = reinterpret_cast<ForceFullGCType>(symbol);
    return true;
}

bool PostTaskWrapper(void* func)
{
    return CJEnvironment::GetInstance()->PostTask(reinterpret_cast<TaskFuncType>(func));
}

bool HasHigherPriorityTaskWrapper()
{
    return CJEnvironment::GetInstance()->HasHigherPriorityTask();
}

CJEnvironment* instance_ = nullptr;
} // namespace

const char *CJEnvironment::cjChipSDKNSName = "cj_chipsdk";
const char *CJEnvironment::cjAppNSName = "moduleNs_default";
const char *CJEnvironment::cjRomSDKNSName = "cj_rom_sdk";
const char *CJEnvironment::cjSysNSName = "default";
const char *CJEnvironment::cjCompatibilitySDKNSName = "cj_compatibility_sdk";
const char *CJEnvironment::cjRuntimeNSName = "cj_runtime";
std::string CJEnvironment::appVersion = "5.1.0.0";
const uint32_t CJEnvironment::majorVersion = 5;
const uint32_t CJEnvironment::minorVersion = 1;
SanitizerKind CJEnvironment::sanitizerKind = SanitizerKind::NONE;

#ifdef WITH_EVENT_HANDLER
static std::shared_ptr<AppExecFwk::EventHandler>GetGHandler()
{
    if (g_handler == nullptr) {
        g_handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    }
    return g_handler;
}
#endif

void CJEnvironment::InitSpawnEnv()
{
#ifdef WITH_EVENT_HANDLER
    GetGHandler();
#endif
}

void CJEnvironment::SetAppPath(const std::string& appPath)
{
    static bool isInited = false;
    static std::mutex initMutex;
    std::lock_guard<std::mutex> lock(initMutex);
    if (isInited) {
        return;
    }
    auto mode = DetectAppNSMode();
    if (instance_) {
        if (instance_->nsMode_ == mode) {
            instance_->InitCJNS(appPath);
            return;
        }
        delete instance_;
    }
    instance_ = new CJEnvironment(mode);
    instance_->InitCJNS(appPath);
    isInited = true;
}

CJEnvironment::CJEnvironment(NSMode mode) : nsMode_(mode)
{
    InitRuntimeNS();
    LoadRuntimeApis();
}

CJEnvironment::~CJEnvironment()
{
    StopRuntime();

    delete lazyApis_;
}

CJEnvironment* CJEnvironment::GetInstance()
{
    return instance_;
}

void CJEnvironment::RegisterCangjieCallback(void* dso)
{
    if (!dso) {
        LOGE("cjruntime is null.");
        return;
    }
    constexpr char CANGJIE_DEBUGGER_LIB_PATH[] = "libark_connect_inspector.z.so";
    auto handlerConnectServerSo = LoadCJLibrary(CJEnvironment::SYSTEM, CANGJIE_DEBUGGER_LIB_PATH);
    if (handlerConnectServerSo == nullptr) {
        LOGE("null handlerConnectServerSo: %{public}s", dlerror());
        return;
    }
    using SendMsgCB = const std::function<void(const std::string& message)>;
    using SetCangjieCallback = void(*)(const std::function<void(const std::string& message, SendMsgCB)>);
    using CangjieCallback = void(*)(const std::string& message, SendMsgCB);
    auto setCangjieCallback = reinterpret_cast<SetCangjieCallback>(
        DynamicFindSymbol(handlerConnectServerSo, "SetCangjieCallback"));
    if (setCangjieCallback == nullptr) {
        LOGE("null setCangjieCallback: %{public}s", dlerror());
        return;
    }
    #define PROFILERAGENT "ProfilerAgent"
    CangjieCallback cangjieCallback = reinterpret_cast<CangjieCallback>(DynamicFindSymbol(dso, PROFILERAGENT));
    if (cangjieCallback == nullptr) {
        dlclose(handlerConnectServerSo);
        handlerConnectServerSo = nullptr;
        LOGE("runtime api not found: %{public}s", PROFILERAGENT);
        return;
    }
    LOGI("find runtime api success");
    setCangjieCallback(cangjieCallback);
    handlerConnectServerSo = nullptr;
}

void* CJEnvironment::LoadRuntimeLib(const char* runtimeLibName) {
    Dl_namespace compatibilitySDK;
    Dl_namespace romSDK;
    dlns_get(cjCompatibilitySDKNSName, &compatibilitySDK);
    dlns_get(cjRomSDKNSName, &romSDK);
    auto dso = DynamicLoadLibrary(&compatibilitySDK, runtimeLibName, 1);
    if (!dso) {
        LOGE("Failed to load %{public}s in ns compatibility.", runtimeLibName);
        dso = DynamicLoadLibrary(&romSDK, runtimeLibName, 1);
    }
    return dso;
}

bool CJEnvironment::LoadRuntimeApis()
{
    if (isRuntimeApiLoaded) {
        return true;
    }
    lazyApis_ = new CJRuntimeAPI();
#ifdef __WINDOWS__
#define RTLIB_NAME "libcangjie-runtime.dll"
#else
#define RTLIB_NAME "libcangjie-runtime.so"
#endif
#ifdef __OHOS__
    auto dso = LoadRuntimeLib(RTLIB_NAME);
#else
    auto dso = DynamicLoadLibrary(RTLIB_NAME, 1);
#endif
    if (!dso) {
        LOGE("load library failed: %{public}s", RTLIB_NAME);
        return false;
    }
#undef RTLIB_NAME
    if (!LoadSymbolInitCJRuntime(dso, *lazyApis_) ||
        !LoadSymbolInitUIScheduler(dso, *lazyApis_) ||
        !LoadSymbolRunUIScheduler(dso, *lazyApis_) ||
        !LoadSymbolFiniCJRuntime(dso, *lazyApis_) ||
        !LoadSymbolInitCJLibrary(dso, *lazyApis_) ||
        !LoadSymbolRegisterEventHandlerCallbacks(dso, *lazyApis_) ||
        !LoadSymbolRegisterStackInfoCallbacks(dso, *lazyApis_) ||
        !LoadSymbolRegisterArkVM(dso, *lazyApis_) ||
        !LoadSymbolDumpHeapSnapshot(dso, *lazyApis_) ||
        !LoadSymbolForceFullGC(dso, *lazyApis_)) {
        LOGE("load symbol failed");
        DynamicFreeLibrary(dso);
        return false;
    }
    RegisterCangjieCallback(dso);
#ifdef __OHOS__
    if (!LoadSymbolRegisterCJUncaughtExceptionHandler(dso, *lazyApis_)) {
        LOGE("load symbol RegisterCJUncaughtExceptionHandler failed");
        DynamicFreeLibrary(dso);
        return false;
    }
#endif
    isRuntimeApiLoaded = true;
    return true;
}

void CJEnvironment::RegisterArkVMInRuntime(unsigned long long externalVM)
{
    if (lazyApis_ == nullptr) {
        return;
    }
    if (lazyApis_->RegisterArkVMInRuntime == nullptr) {
        return;
    }
    lazyApis_->RegisterArkVMInRuntime(externalVM);
}

void CJEnvironment::RegisterStackInfoCallbacks(UpdateStackInfoFuncType uFunc)
{
    if (lazyApis_ == nullptr) {
        return;
    }
    if (lazyApis_->RegisterStackInfoCallbacks == nullptr) {
        return;
    }
    lazyApis_->RegisterStackInfoCallbacks(uFunc);
}

void CJEnvironment::RegisterCJUncaughtExceptionHandler(const CJUncaughtExceptionInfo& handle)
{
    if (lazyApis_ == nullptr) {
        return;
    }
    if (lazyApis_->RegisterCJUncaughtExceptionHandler == nullptr) {
        return;
    }
    lazyApis_->RegisterCJUncaughtExceptionHandler(handle);
}

void CJEnvironment::RegisterEventHandlerCallbacks()
{
    if (lazyApis_ == nullptr) {
        return;
    }
    if (lazyApis_->RegisterEventHandlerCallbacks == nullptr) {
        return;
    }
    lazyApis_->RegisterEventHandlerCallbacks(PostTaskWrapper, HasHigherPriorityTaskWrapper);
}

int CJEnvironment::InitCJRuntime(const struct RuntimeParam* rtParams)
{
    if (lazyApis_ == nullptr) {
        return E_FAILED;
    }
    if (lazyApis_->InitCJRuntime == nullptr) {
        return E_FAILED;
    }
    return lazyApis_->InitCJRuntime(rtParams);
}

void CJEnvironment::DumpHeapSnapshot(int fd)
{
    if (lazyApis_ == nullptr) {
        return;
    }
    if (lazyApis_->DumpHeapSnapshot == nullptr) {
        return;
    }
    lazyApis_->DumpHeapSnapshot(fd);
}

void CJEnvironment::ForceFullGC()
{
    if (lazyApis_ == nullptr) {
        return;
    }
    if (lazyApis_->ForceFullGC == nullptr) {
        return;
    }
    lazyApis_->ForceFullGC();
}

bool CJEnvironment::PostTask(TaskFuncType task)
{
#ifdef WITH_EVENT_HANDLER
    if (task == nullptr) {
        LOGE("null task could not be posted");
        return false;
    }

    bool postDone = GetGHandler()->PostTask(task, "spawn-main-task-from-cj", 0, AppExecFwk::EventQueue::Priority::HIGH);
    if (!postDone) {
        LOGE("event handler support cj ui scheduler");
        return false;
    }
    return true;
#endif
    return true;
}

bool CJEnvironment::HasHigherPriorityTask()
{
#ifdef WITH_EVENT_HANDLER
    return GetGHandler()->HasPreferEvent(static_cast<int>(AppExecFwk::EventQueue::Priority::HIGH));
#endif
    return false;
}

void CJEnvironment::InitCJChipSDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJChipSDKNS: %{public}s", path.c_str());
    Dl_namespace chip_sdk;
    DynamicInitNamespace(&chip_sdk, path.c_str(), CJEnvironment::cjChipSDKNSName);
#endif
}

// Init app namespace
void CJEnvironment::InitCJAppNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJAppNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjAppNSName);
    DynamicInherit(&ns, CJEnvironment::cjRuntimeNSName, "allow_all_shared_libs");
    DynamicInherit(&ns, CJEnvironment::cjCompatibilitySDKNSName, "allow_all_shared_libs");
    DynamicInherit(&ns, CJEnvironment::cjRomSDKNSName, "allow_all_shared_libs");
    if (nsMode_ == NSMode::APP) {
        DynamicInherit(&ns, CJEnvironment::cjChipSDKNSName, "libssl_openssl.z.so");
    }
#endif
}

void CJEnvironment::InitCJRomSDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJRomSDKNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjRomSDKNSName);
    DynamicInherit(&ns, CJEnvironment::cjRuntimeNSName, "allow_all_shared_libs");
#endif
}

void CJEnvironment::InitCJCompatibilitySDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJCompatibilitySDKNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjCompatibilitySDKNSName);
    DynamicInherit(&ns, CJEnvironment::cjRuntimeNSName, "allow_all_shared_libs");
#endif
}

void CJEnvironment::InitCJRuntimeNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJRuntimeNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjRuntimeNSName);
#endif
}

bool CJEnvironment::StartRuntime()
{
    if (isRuntimeStarted_) {
        return true;
    }

    if (!LoadRuntimeApis()) {
        LOGE("LoadRuntimeApis failed");
        return false;
    }

    RuntimeParam rtParams {
        .heapParam = {
            .regionSize = 64,
            .heapSize = 256 * 1024,
            .exemptionThreshold= 0.8,
            .heapUtilization = 0.8,
            .heapGrowth = 0.15,
            .allocationRate = 0,
            .allocationWaitTime = 0,
        },
        .gcParam = {
            .gcThreshold = 0,
            .garbageThreshold = 0,
            .gcInterval = 0,
            .backupGCInterval = 0,
            .gcThreads = 0,
        },
        .logParam = {
            .logLevel = RTLOG_ERROR,
        },
        .coParam = {
            .thStackSize = 2 * 1024,
            .coStackSize = 2 * 1024,
            .processorNum = 8,
        }
    };
    auto status = InitCJRuntime(&rtParams);
    if (status != E_OK) {
        LOGE("init cj runtime failed: %{public}d", status);
        return false;
    }
    RegisterEventHandlerCallbacks();
    isRuntimeStarted_ = true;
    return true;
}

void CJEnvironment::StopRuntime()
{
    if (!isRuntimeStarted_) {
        return;
    }

    if (isUISchedulerStarted_) {
        StopUIScheduler();
    }

    auto code = lazyApis_->FiniCJRuntime();
    if (code == E_OK) {
        isRuntimeStarted_ = false;
    }
}

bool CJEnvironment::StartUIScheduler()
{
    if (isUISchedulerStarted_) {
        return true;
    }

    uiScheduler_ = lazyApis_->InitUIScheduler();
    if (!uiScheduler_) {
        LOGE("init cj ui scheduler failed");
        return false;
    }

    isUISchedulerStarted_ = true;
    return true;
}

void CJEnvironment::StopUIScheduler()
{
    isUISchedulerStarted_ = false;
}

void* CJEnvironment::LoadCJLibrary(const char* dlName)
{
    if (!StartRuntime()) {
        LOGE("StartRuntime failed");
        return nullptr;
    }
    auto handle = LoadCJLibrary(APP, dlName);
    if (!handle) {
        LOGE("load cj library failed: %{public}s", DynamicGetError());
        return nullptr;
    }

    LOGI("LoadCJLibrary InitCJLibrary: %{public}s", dlName);
    auto status = lazyApis_->InitCJLibrary(dlName);
    if (status != E_OK) {
        LOGE("InitCJLibrary failed: %{public}s", dlName);
        UnLoadCJLibrary(handle);
        return nullptr;
    }
    isLoadCJLibrary_ = true;
    return handle;
}

void* CJEnvironment::LoadCJLibrary(OHOS::CJEnvironment::LibraryKind kind, const char* dlName)
{
#ifdef __OHOS__
    Dl_namespace ns;
    Dl_namespace sdk;
    switch (kind) {
        case APP:
            dlns_get(CJEnvironment::cjAppNSName, &ns);
            break;
        case SYSTEM:
            dlns_get(CJEnvironment::cjSysNSName, &ns);
            break;
        case SDK:
            dlns_get(CJEnvironment::cjCompatibilitySDKNSName, &ns);
            dlns_get(CJEnvironment::cjRomSDKNSName, &sdk);
            break;
    }
    auto handle = DynamicLoadLibrary(&ns, dlName, 0);
    if (!handle && kind == SDK) {
        LOGE("Failed to load %{public}s in ns compatibility.", dlName);
        handle = DynamicLoadLibrary(&sdk, dlName, 0);
    }
#else
    auto handle = DynamicLoadLibrary(dlName, 1);
#endif
    if (!handle) {
        LOGE("load cj library failed: %{public}s", DynamicGetError());
        return nullptr;
    }
    isLoadCJLibrary_ = true;
    return handle;
}

bool CJEnvironment::CheckLoadCJLibrary()
{
    return isLoadCJLibrary_;
}

void CJEnvironment::UnLoadCJLibrary(void* handle)
{
    DynamicFreeLibrary(handle);
}

void* CJEnvironment::GetSymbol(void* dso, const char* symbol)
{
    return DynamicFindSymbol(dso, symbol);
}

bool CJEnvironment::StartDebugger()
{
#ifdef __OHOS__
    Dl_namespace ns;
    dlns_get(CJEnvironment::cjSysNSName, &ns);
    auto handle = DynamicLoadLibrary(&ns, DEBUGGER_LIBNAME, 0);
#else
    auto handle = DynamicLoadLibrary(DEBUGGER_LIBNAME, 0);
#endif
    if (!handle) {
        LOGE("failed to load library: %{public}s", DEBUGGER_LIBNAME);
        return false;
    }
    auto symbol = DynamicFindSymbol(handle, DEBUGGER_SYMBOL_NAME);
    if (!symbol) {
        LOGE("failed to find symbol: %{public}s", DEBUGGER_SYMBOL_NAME);
        DynamicFreeLibrary(handle);
        return false;
    }
    auto func = reinterpret_cast<bool (*)(int, const std::string&)>(symbol);
    std::string name = "PandaDebugger";
    func(0, name);
    return true;
}

std::vector<uint32_t> SplitVersion(std::string& version, char separator)
{
    std::vector<uint32_t> result;
    std::stringstream ss(version);
    std::string item;
    uint32_t num;
    while (std::getline(ss, item, separator)) {
        auto res = std::from_chars(item.data(), item.data() + item.size(), num);
        if (res.ec == std::errc()) {
            result.push_back(num);
        } else {
            LOGE("Incorrect version");
            return result;
        }
    }
    return result;
}

CJEnvironment::NSMode CJEnvironment::DetectAppNSMode()
{
    LOGI("App compileSDKVersion is %{public}s", CJEnvironment::appVersion.c_str());
    std::vector<uint32_t> tokens = SplitVersion(CJEnvironment::appVersion, '.');
    if (tokens.size() <= 1) {
        return NSMode::SINK;
    }
    if (tokens[0] > CJEnvironment::majorVersion ||
        (tokens[0] == CJEnvironment::majorVersion && tokens[1] >= CJEnvironment::minorVersion)) {
        return NSMode::SINK;
    } else {
        return NSMode::APP;
    }
}

void CJEnvironment::InitRuntimeNS()
{
#ifdef __OHOS__
    switch (CJEnvironment::sanitizerKind) {
        case SanitizerKind::ASAN:
            InitCJRuntimeNS(CJ_ASAN_PATH);
            break;
        default:
            InitCJRuntimeNS(CJ_RUNTIME_PATH);
    }
    if (nsMode_ == NSMode::APP) {
        InitCJChipSDKNS(CJ_CHIPSDK_PATH);
    }
    InitCJRomSDKNS(CJ_SDK_PATH);
    InitCJCompatibilitySDKNS(CJ_RT_PATH + ":" + CJ_LIB_PATH);
#endif
}

void CJEnvironment::InitCJNS(const std::string& appPath)
{
#ifdef __OHOS__
    InitCJAppNS(appPath.empty() ? SANDBOX_LIB_PATH : appPath);
#endif
    if (!StartRuntime()) {
        LOGE("Failed to start cj runtime.");
        delete lazyApis_;
        lazyApis_ = nullptr;
        return;
    }
    StartUIScheduler();
}

void CJEnvironment::SetAppVersion(std::string& version)
{
    CJEnvironment::appVersion = version;
}

void CJEnvironment::SetSanitizerKindRuntimeVersion(SanitizerKind kind)
{
    LOGI("Set sanitizer for cj.");
    CJEnvironment::sanitizerKind = kind;
}

CJEnvMethods* CJEnvironment::CreateEnvMethods()
{
    static CJEnvMethods gCJEnvMethods {
        .initCJAppNS = [](const std::string& path) {
            // to keep compatibility with older version
            CJEnvironment::SetAppPath(path);
        },
        .initCJSDKNS = [](const std::string& path) {
            // @deprecated
        },
        .initCJSysNS = [](const std::string& path) {
            // @deprecated
        },
        .initCJChipSDKNS = [](const std::string& path) {
            // @deprecated
        },
        .startRuntime = [] {
            return CJEnvironment::GetInstance()->StartRuntime();
        },
        .startUIScheduler = [] {
            return CJEnvironment::GetInstance()->StartUIScheduler();
        },
        .loadCJModule = [](const char* dllName) {
            return CJEnvironment::GetInstance()->LoadCJLibrary(dllName);
        },
        .loadLibrary = [](uint32_t kind, const char* dllName) {
            return CJEnvironment::GetInstance()->LoadCJLibrary(static_cast<CJEnvironment::LibraryKind>(kind), dllName);
        },
        .getSymbol = [](void* handle, const char* dllName) {
            return CJEnvironment::GetInstance()->GetSymbol(handle, dllName);
        },
        .loadCJLibrary = [](const char* dllName) {
            return CJEnvironment::GetInstance()->LoadCJLibrary(dllName);
        },
        .startDebugger = []() {
            return CJEnvironment::GetInstance()->StartDebugger();
        },
        .registerCJUncaughtExceptionHandler = [](const CJUncaughtExceptionInfo& handle) {
            return CJEnvironment::GetInstance()->RegisterCJUncaughtExceptionHandler(handle);
        },
        .setSanitizerKindRuntimeVersion = [](SanitizerKind kind) {
            CJEnvironment::SetSanitizerKindRuntimeVersion(kind);
        },
        .checkLoadCJLibrary = []() {
            return CJEnvironment::GetInstance()->CheckLoadCJLibrary();
        },
        .registerArkVMInRuntime = [](unsigned long long arkVM) {
            CJEnvironment::GetInstance()->RegisterArkVMInRuntime(arkVM);
        },
        .registerStackInfoCallbacks = [](UpdateStackInfoFuncType uFunc) {
            CJEnvironment::GetInstance()->RegisterStackInfoCallbacks(uFunc);
        },
        .setAppVersion = [](std::string& version) {
            CJEnvironment::SetAppVersion(version);
        },
        .dumpHeapSnapshot = [](int fd) {
            CJEnvironment::GetInstance()->DumpHeapSnapshot(fd);
        },
        .forceFullGC = []() {
            CJEnvironment::GetInstance()->ForceFullGC();
        }
    };
    return &gCJEnvMethods;
}

CJ_EXPORT extern "C" void OHOS_InitSpawnEnv()
{
    CJEnvironment::InitSpawnEnv();
}

CJ_EXPORT extern "C" CJEnvMethods* OHOS_GetCJEnvInstance()
{
    return CJEnvironment::CreateEnvMethods();
}
}
