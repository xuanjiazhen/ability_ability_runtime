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

#include <gtest/gtest.h>
#include <memory>
#include <string>

#define private public
#define protected public
#include "interop_object_instance.h"
#undef private
#undef protected

#include "runtime.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class MockInteropRuntime : public Runtime {
public:
    MockInteropRuntime() = default;
    ~MockInteropRuntime() = default;

    Language GetLanguage() const override
    {
        return language_;
    }

    void SetLanguage(Language lang)
    {
        language_ = lang;
    }

    void StartDebugMode(const DebugOption debugOption) override {}
    void FinishPreload() override {}
    bool LoadRepairPatch(const std::string& patchFile, const std::string& baseFile) override { return true; }
    bool NotifyHotReloadPage() override { return true; }
    bool SuspendVM(uint32_t tid) override { return true; }
    void ResumeVM(uint32_t tid) override {}
    bool UnLoadRepairPatch(const std::string& patchFile) override { return true; }
    void DumpHeapSnapshot(bool isPrivate) override {}
    void DumpCpuProfile() override {}
    void DestroyHeapProfiler() override {}
    void ForceFullGC() override {}
    void AllowCrossThreadExecution() override {}
    void GetHeapPrepare() override {}
    void NotifyApplicationState(bool isBackground) override {}
    void PreloadSystemModule(const std::string& moduleName) override {}
    void PreloadMainAbility(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath, bool isEsMode, const std::string& srcEntrance) override {}
    void PreloadModule(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath, bool isEsMode, bool useCommonTrunk) override {}
    bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false) override { return true; }
    bool Initialize(const Options& options) override { return true; }
    void Deinitialize() override {}
    napi_value LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk = false) override { return nullptr; }
    napi_value LoadJsModule(const std::string& path, const std::string& hapPath) override { return nullptr; }
    bool LoadScript(const std::string& path, std::vector<uint8_t>* buffer = nullptr, bool isBundle = false) override { return true; }
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath) override {}
    void SetDeviceDisconnectCallback(const std::function<bool()> &cb) override {}
    void StartProfiler(const DebugOption debugOption) override {}
    void SetExtensionApiCheckCallback(
        std::function<bool(const std::string &className, const std::string &fileName)> &cb) override {}
    void DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary = false) override {}
    void DumpHeapSnapshot(uint32_t tid, const Runtime::JsHeapDumpParam &param) override {}
    void ForceFullGC(uint32_t tid) override {}

private:
    Language language_ = static_cast<Language>(100);
};

class InteropObjectInstanceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number: InteropObjectInstanceTest_CreateInteropObject_0100
 * @tc.name: CreateInteropObject with unsupported language
 * @tc.desc: Verify CreateInteropObject returns nullptr when runtime language is neither JS nor ETS.
 */
HWTEST_F(InteropObjectInstanceTest, InteropObjectInstanceTest_CreateInteropObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateInteropObject_0100 start";
    MockInteropRuntime runtime;
    runtime.SetLanguage(static_cast<Runtime::Language>(999));
    AbilityLifecycleCallbackArgs arg;
    auto result = InteropObjectInstance::CreateInteropObject(runtime, arg);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateInteropObject_0100 end";
}

/**
 * @tc.number: InteropObjectInstanceTest_CreateInteropObject_0200
 * @tc.name: CreateInteropObject with JS language triggers dlopen
 * @tc.desc: Verify CreateInteropObject with JS language attempts dlopen (returns null when .so not available).
 */
HWTEST_F(InteropObjectInstanceTest, InteropObjectInstanceTest_CreateInteropObject_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateInteropObject_0200 start";
    MockInteropRuntime runtime;
    runtime.SetLanguage(Runtime::Language::JS);
    AbilityLifecycleCallbackArgs arg;
    auto result = InteropObjectInstance::CreateInteropObject(runtime, arg);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateInteropObject_0200 end";
}

/**
 * @tc.number: InteropObjectInstanceTest_CreateInteropObject_0300
 * @tc.name: CreateInteropObject with ETS language triggers dlopen
 * @tc.desc: Verify CreateInteropObject with ETS language attempts dlopen (returns null when .so not available).
 */
HWTEST_F(InteropObjectInstanceTest, InteropObjectInstanceTest_CreateInteropObject_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateInteropObject_0300 start";
    MockInteropRuntime runtime;
    runtime.SetLanguage(Runtime::Language::ETS);
    AbilityLifecycleCallbackArgs arg;
    auto result = InteropObjectInstance::CreateInteropObject(runtime, arg);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateInteropObject_0300 end";
}

/**
 * @tc.number: InteropObjectInstanceTest_CreateJsInteropAbilityLifecycleCallback_0100
 * @tc.name: CreateJsInteropAbilityLifecycleCallback with null env
 * @tc.desc: Verify CreateJsInteropAbilityLifecycleCallback returns nullptr when dlopen fails.
 */
HWTEST_F(InteropObjectInstanceTest, InteropObjectInstanceTest_CreateJsInteropAbilityLifecycleCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateJsInteropAbilityLifecycleCallback_0100 start";
    auto result = InteropObjectInstance::CreateJsInteropAbilityLifecycleCallback(nullptr);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "InteropObjectInstanceTest_CreateJsInteropAbilityLifecycleCallback_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
