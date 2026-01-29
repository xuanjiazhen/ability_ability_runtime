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

#include <gtest/gtest.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

#include "interop_object_instance.h"

using namespace testing;
using namespace testing::ext;

namespace {
std::atomic<int> g_dlopenCalls = 0;
std::atomic<int> g_dlsymCalls = 0;
std::atomic<int> g_dlcloseCalls = 0;
std::atomic<bool> g_dlopenFail = false;
std::atomic<bool> g_dlsymFail = false;
void *g_mockHandle = reinterpret_cast<void *>(0x1);
const char *g_lastDlError = "mock_dl_error";
}

extern "C" void *dlopen(const char *filename, int)
{
    (void)filename;
    g_dlopenCalls++;
    if (g_dlopenFail.load()) {
        return nullptr;
    }
    return g_mockHandle;
}

namespace OHOS {
namespace AbilityRuntime {
class DummyInteropObject : public InteropObject {
public:
    explicit DummyInteropObject(bool fromNapi) : fromNapi_(fromNapi) {}
    bool IsFromNapi() override { return fromNapi_; }
    bool IsFromAni() override { return !fromNapi_; }

private:
    bool fromNapi_;
};

class DummyInteropAbilityLifecycleCallback : public InteropAbilityLifecycleCallback {
public:
    int32_t Register(void *) override { return 0; }
    bool Unregister(void *) override { return true; }
    bool Empty() override { return false; }
};

static InteropObject *CreateEtsInteropObjectMock(const Runtime &, const AbilityLifecycleCallbackArgs &)
{
    return new DummyInteropObject(false);
}

static InteropObject *CreateJsInteropObjectMock(const Runtime &, const AbilityLifecycleCallbackArgs &)
{
    return new DummyInteropObject(true);
}

static InteropAbilityLifecycleCallback *CreateJsInteropAbilityLifecycleCallbackMock(void *)
{
    return new DummyInteropAbilityLifecycleCallback();
}
} // namespace AbilityRuntime
} // namespace OHOS

extern "C" void *dlsym(void *, const char *symbol)
{
    g_dlsymCalls++;
    if (g_dlsymFail.load()) {
        return nullptr;
    }
    if (symbol == nullptr) {
        return nullptr;
    }
    if (std::string(symbol) == "OHOS_CreateEtsInteropObject") {
        return reinterpret_cast<void *>(&OHOS::AbilityRuntime::CreateEtsInteropObjectMock);
    }
    if (std::string(symbol) == "OHOS_CreateJsInteropObject") {
        return reinterpret_cast<void *>(&OHOS::AbilityRuntime::CreateJsInteropObjectMock);
    }
    if (std::string(symbol) == "OHOS_CreateJsInteropAbilityLifecycleCallback") {
        return reinterpret_cast<void *>(
            &OHOS::AbilityRuntime::CreateJsInteropAbilityLifecycleCallbackMock);
    }
    return nullptr;
}

extern "C" int dlclose(void *)
{
    g_dlcloseCalls++;
    return 0;
}

extern "C" char *dlerror(void)
{
    return const_cast<char *>(g_lastDlError);
}

namespace OHOS {
namespace AbilityRuntime {
namespace {
using CreateInteropObjectFunc = InteropObject *(*)(const Runtime &, const AbilityLifecycleCallbackArgs &);
using CreateInteropAbilityLifecycleCallbackFunc = InteropAbilityLifecycleCallback *(*)(void *);

extern CreateInteropObjectFunc g_etsInteropObjectCreateFunc;
extern std::mutex g_etsInteropObjectCreateFuncMutex;
extern CreateInteropObjectFunc g_jsInteropObjectCreateFunc;
extern std::mutex g_jsInteropObjectCreateFuncMutex;
extern CreateInteropAbilityLifecycleCallbackFunc g_jsInteropAbilityLifecycleCallbackCreateFunc;
extern std::mutex g_jsInteropAbilityLifecycleCallbackCreateFuncMutex;
extern void *g_handle;
extern std::mutex g_handleMutex;

class DummyRuntime : public Runtime {
public:
    explicit DummyRuntime(Language lang) : lang_(lang) {}
    Language GetLanguage() const override { return lang_; }

    void StartDebugMode(const DebugOption) override {}
    void DumpHeapSnapshot(bool) override {}
    void DumpCpuProfile() override {}
    void DestroyHeapProfiler() override {}
    void ForceFullGC() override {}
    void ForceFullGC(uint32_t) override {}
    void DumpHeapSnapshot(uint32_t, bool, bool) override {}
    void AllowCrossThreadExecution() override {}
    void GetHeapPrepare() override {}
    void NotifyApplicationState(bool) override {}
    bool SuspendVM(uint32_t) override { return false; }
    void ResumeVM(uint32_t) override {}
    void PreloadSystemModule(const std::string &) override {}
    void PreloadMainAbility(const std::string &, const std::string &, const std::string &, bool, const std::string &) override {}
    void PreloadModule(const std::string &, const std::string &, const std::string &, bool, bool) override {}
    void FinishPreload() override {}
    bool LoadRepairPatch(const std::string &, const std::string &) override { return false; }
    bool NotifyHotReloadPage() override { return false; }
    bool UnLoadRepairPatch(const std::string &) override { return false; }
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string> &) override {}
    void StartProfiler(const DebugOption) override {}
    void SetDeviceDisconnectCallback(const std::function<bool()> &) override {}

private:
    Language lang_;
};

void ResetInteropGlobals()
{
    std::lock_guard<std::mutex> handleLock(g_handleMutex);
    g_handle = nullptr;

    std::lock_guard<std::mutex> etsLock(g_etsInteropObjectCreateFuncMutex);
    g_etsInteropObjectCreateFunc = nullptr;

    std::lock_guard<std::mutex> jsLock(g_jsInteropObjectCreateFuncMutex);
    g_jsInteropObjectCreateFunc = nullptr;

    std::lock_guard<std::mutex> cbLock(g_jsInteropAbilityLifecycleCallbackCreateFuncMutex);
    g_jsInteropAbilityLifecycleCallbackCreateFunc = nullptr;

    g_dlopenCalls = 0;
    g_dlsymCalls = 0;
    g_dlcloseCalls = 0;
    g_dlopenFail = false;
    g_dlsymFail = false;
}
}

class InteropObjectInstanceTest : public testing::Test {
public:
    void SetUp() override
    {
        ResetInteropGlobals();
    }
};

/**
 * @tc.number: CreateInteropObject_0100
 * @tc.name: CreateInteropObject
 * @tc.desc: Verify unknown language returns nullptr.
 */
HWTEST_F(InteropObjectInstanceTest, CreateInteropObject_0100, Function | MediumTest | Level1)
{
    DummyRuntime runtime(Runtime::Language::UNKNOWN);
    AbilityLifecycleCallbackArgs args;
    auto result = InteropObjectInstance::CreateInteropObject(runtime, args);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: CreateInteropObject_0200
 * @tc.name: CreateInteropObject
 * @tc.desc: Verify JS language uses ETS creation path.
 */
HWTEST_F(InteropObjectInstanceTest, CreateInteropObject_0200, Function | MediumTest | Level1)
{
    DummyRuntime runtime(Runtime::Language::JS);
    AbilityLifecycleCallbackArgs args;

    g_dlopenFail = true;
    auto result = InteropObjectInstance::CreateInteropObject(runtime, args);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(g_dlopenCalls.load(), 1);
}

/**
 * @tc.number: CreateInteropObject_0300
 * @tc.name: CreateInteropObject
 * @tc.desc: Verify ETS language uses JS creation path.
 */
HWTEST_F(InteropObjectInstanceTest, CreateInteropObject_0300, Function | MediumTest | Level1)
{
    DummyRuntime runtime(Runtime::Language::ETS);
    AbilityLifecycleCallbackArgs args;

    g_dlopenFail = true;
    auto result = InteropObjectInstance::CreateInteropObject(runtime, args);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(g_dlopenCalls.load(), 1);
}

/**
 * @tc.number: CreateInteropObject_0400
 * @tc.name: CreateInteropObject
 * @tc.desc: Verify dlsym failure closes handle and returns nullptr.
 */
HWTEST_F(InteropObjectInstanceTest, CreateInteropObject_0400, Function | MediumTest | Level1)
{
    DummyRuntime runtime(Runtime::Language::JS);
    AbilityLifecycleCallbackArgs args;

    g_dlsymFail = true;
    auto result = InteropObjectInstance::CreateInteropObject(runtime, args);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(g_dlopenCalls.load(), 1);
    EXPECT_EQ(g_dlsymCalls.load(), 1);
    EXPECT_EQ(g_dlcloseCalls.load(), 1);
}

/**
 * @tc.number: CreateInteropObject_0500
 * @tc.name: CreateInteropObject
 * @tc.desc: Verify dlsym success caches creator.
 */
HWTEST_F(InteropObjectInstanceTest, CreateInteropObject_0500, Function | MediumTest | Level1)
{
    DummyRuntime runtime(Runtime::Language::JS);
    AbilityLifecycleCallbackArgs args;

    auto result1 = InteropObjectInstance::CreateInteropObject(runtime, args);
    EXPECT_NE(result1, nullptr);
    EXPECT_EQ(g_dlopenCalls.load(), 1);
    EXPECT_EQ(g_dlsymCalls.load(), 1);

    g_dlsymFail = true;
    auto result2 = InteropObjectInstance::CreateInteropObject(runtime, args);
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(g_dlsymCalls.load(), 1);
}

/**
 * @tc.number: CreateJsInteropAbilityLifecycleCallback_0100
 * @tc.name: CreateJsInteropAbilityLifecycleCallback
 * @tc.desc: Verify dlsym failure returns nullptr.
 */
HWTEST_F(InteropObjectInstanceTest, CreateJsInteropAbilityLifecycleCallback_0100, Function | MediumTest | Level1)
{
    g_dlsymFail = true;
    auto result = InteropObjectInstance::CreateJsInteropAbilityLifecycleCallback(nullptr);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(g_dlopenCalls.load(), 1);
    EXPECT_EQ(g_dlsymCalls.load(), 1);
}

/**
 * @tc.number: CreateJsInteropAbilityLifecycleCallback_0200
 * @tc.name: CreateJsInteropAbilityLifecycleCallback
 * @tc.desc: Verify dlsym success caches creator.
 */
HWTEST_F(InteropObjectInstanceTest, CreateJsInteropAbilityLifecycleCallback_0200, Function | MediumTest | Level1)
{
    auto result1 = InteropObjectInstance::CreateJsInteropAbilityLifecycleCallback(nullptr);
    EXPECT_NE(result1, nullptr);
    EXPECT_EQ(g_dlsymCalls.load(), 1);

    g_dlsymFail = true;
    auto result2 = InteropObjectInstance::CreateJsInteropAbilityLifecycleCallback(nullptr);
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(g_dlsymCalls.load(), 1);
}
} // namespace AbilityRuntime
} // namespace OHOS

#include "interop_object_instance.cpp"
