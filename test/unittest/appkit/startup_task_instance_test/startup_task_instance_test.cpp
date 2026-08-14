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
#include "startup_task_instance.h"
#undef private
#undef protected

#include "runtime.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class MockRuntimeForTaskInstance : public Runtime {
public:
    MockRuntimeForTaskInstance() = default;
    ~MockRuntimeForTaskInstance() = default;
    Language GetLanguage() const override { return Runtime::Language::JS; }
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
    bool RunScript(
        const std::string& path, const std::string& hapPath, bool useCommonChunk = false) override { return true; }
    bool Initialize(const Options& options) override { return true; }
    void Deinitialize() override {}
    napi_value LoadJsBundle(
        const std::string& path, const std::string& hapPath, bool useCommonChunk = false) override { return nullptr; }
    napi_value LoadJsModule(const std::string& path, const std::string& hapPath) override { return nullptr; }
    bool LoadScript(const std::string& path,
        std::vector<uint8_t>* buffer = nullptr, bool isBundle = false) override { return true; }
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath) override {}
    void SetDeviceDisconnectCallback(const std::function<bool()> &cb) override {}
    void StartProfiler(const DebugOption debugOption) override {}
    void SetExtensionApiCheckCallback(
        std::function<bool(const std::string &className, const std::string &fileName)> &cb) override {}
    void DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary = false) override {}
    void DumpHeapSnapshot(uint32_t tid, const Runtime::JsHeapDumpParam &param) override {}
    void ForceFullGC(uint32_t tid) override {}
};

class StartupTaskInstanceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number: StartupTaskInstanceTest_GetSpecifiedRuntime_0100
 * @tc.name: GetSpecifiedRuntime with null runtime
 * @tc.desc: Verify GetSpecifiedRuntime returns the input runtime (null) when runtime is nullptr.
 */
HWTEST_F(StartupTaskInstanceTest, StartupTaskInstanceTest_GetSpecifiedRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskInstanceTest_GetSpecifiedRuntime_0100 start";
    std::unique_ptr<Runtime> runtime = nullptr;
    auto &result = StartupTaskInstance::GetSpecifiedRuntime(runtime, CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(result.get(), nullptr);
    GTEST_LOG_(INFO) << "StartupTaskInstanceTest_GetSpecifiedRuntime_0100 end";
}

/**
 * @tc.number: StartupTaskInstanceTest_GetSpecifiedRuntime_0200
 * @tc.name: GetSpecifiedRuntime with ARKTS_1_2 mode
 * @tc.desc: Verify GetSpecifiedRuntime returns the input runtime when arkTSMode is "static" (1.2).
 */
HWTEST_F(StartupTaskInstanceTest, StartupTaskInstanceTest_GetSpecifiedRuntime_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskInstanceTest_GetSpecifiedRuntime_0200 start";
    auto runtime = std::make_unique<MockRuntimeForTaskInstance>();
    ASSERT_NE(runtime, nullptr);
    auto &result = StartupTaskInstance::GetSpecifiedRuntime(runtime, CODE_LANGUAGE_ARKTS_1_2);
    EXPECT_EQ(result.get(), runtime.get());
    GTEST_LOG_(INFO) << "StartupTaskInstanceTest_GetSpecifiedRuntime_0200 end";
}

/**
 * @tc.number: StartupTaskInstanceTest_CreateStartupTask_0100
 * @tc.name: CreateStartupTask with null runtime
 * @tc.desc: Verify CreateStartupTask returns nullptr when runtime is null.
 */
HWTEST_F(StartupTaskInstanceTest, StartupTaskInstanceTest_CreateStartupTask_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskInstanceTest_CreateStartupTask_0100 start";
    std::unique_ptr<Runtime> runtime = nullptr;
    StartupTaskInfo info;
    auto result = StartupTaskInstance::CreateStartupTask(runtime, CODE_LANGUAGE_ARKTS_1_0, info, false);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "StartupTaskInstanceTest_CreateStartupTask_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
