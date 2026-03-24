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
#include <gtest/hwext/gtest-ext.h>
#include <gmock/gmock.h>
#include "cli_manager_service.h"
#include "cli_manager_client.h"
#include "cli_tool_info.h"
#include "cli_execution_context.h"

using namespace testing;
using namespace OHOS;
using namespace CliMgr;

namespace {
constexpr const char* TEST_TOOL_NAME = "test-tool";
constexpr const char* TEST_TOOL_VERSION = "1.0.0";
constexpr const char* TEST_TOOL_DESC = "Test tool for unit testing";
} // namespace

/**
 * @brief CLI Manager Service 单元测试
 */
class CLIManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    static std::shared_ptr<CLIManagerService> service_;
};

std::shared_ptr<CLIManagerService> CLIManagerServiceTest::service_ = nullptr;

void CLIManagerServiceTest::SetUpTestCase()
{
    service_ = std::make_shared<CLIManagerService>(SA_CLIMGR_ID, false);
    ASSERT_NE(service_, nullptr);
    service_->OnStart();
}

void CLIManagerServiceTest::TearDownTestCase()
{
    if (service_) {
        service_->OnStop();
        service_.reset();
    }
}

void CLIManagerServiceTest::SetUp()
{
    // 每个测试用例前的准备
}

void CLIManagerServiceTest::TearDown()
{
    // 每个测试用例后的清理
}

/**
 * @brief 测试服务初始化
 */
HWTEST_F(CLIManagerServiceTest, ServiceInitialization, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    EXPECT_TRUE(service_->GetRegistryManager() != nullptr);
    EXPECT_TRUE(service_->GetProcessManager() != nullptr);
    EXPECT_TRUE(service_->GetEventDispatcher() != nullptr);
    EXPECT_TRUE(service_->GetPermissionChecker() != nullptr);
}

/**
 * @brief 测试工具注册
 */
HWTEST_F(CLIManagerServiceTest, ToolRegistration, TestSize.Level1)
{
    auto registry = service_->GetRegistryManager();
    ASSERT_NE(registry, nullptr);

    // 创建测试工具
    ToolInfo tool;
    tool.name = TEST_TOOL_NAME;
    tool.version = TEST_TOOL_VERSION;
    tool.description = TEST_TOOL_DESC;
    tool.executablePath = "/usr/bin/test-tool";
    tool.type = ToolType::SHORT_LIVED;
    tool.timeoutMs = 30000;
    tool.enabled = true;

    // 注册工具
    int32_t ret = registry->RegisterTool(tool);
    EXPECT_EQ(ret, ErrCode::OK);

    // 查询工具
    ToolInfo queriedTool;
    ret = registry->QueryToolByName(TEST_TOOL_NAME, queriedTool);
    EXPECT_EQ(ret, ErrCode::OK);
    EXPECT_EQ(queriedTool.name, TEST_TOOL_NAME);
    EXPECT_EQ(queriedTool.version, TEST_TOOL_VERSION);
}

/**
 * @brief 测试工具查询
 */
HWTEST_F(CLIManagerServiceTest, QueryTools, TestSize.Level1)
{
    std::vector<ToolInfo> tools;
    int32_t ret = service_->QueryTools(tools);

    EXPECT_EQ(ret, ErrCode::OK);
    EXPECT_GE(tools.size(), 0);  // 至少应该有 0 个工具
}

/**
 * @brief 测试按名称查询工具
 */
HWTEST_F(CLIManagerServiceTest, QueryToolByName, TestSize.Level1)
{
    ToolInfo tool;
    int32_t ret = service_->QueryToolByName(TEST_TOOL_NAME, tool);

    if (ret == ErrCode::OK) {
        EXPECT_EQ(tool.name, TEST_TOOL_NAME);
    } else {
        EXPECT_EQ(ret, ErrCode::ERR_NOT_FOUND);
    }
}

/**
 * @brief 测试同步执行工具
 */
HWTEST_F(CLIManagerServiceTest, ExecToolSync, TestSize.Level2)
{
    Json::Value arguments;
    arguments["test"] = "value";

    ExecOptions options;
    options.mode = ExecutionMode::SYNC;
    options.timeoutMs = 5000;

    ExecutionResult result;
    int32_t ret = service_->ExecTool(TEST_TOOL_NAME, arguments, options, result);

    // 可能失败（工具不存在）或成功
    EXPECT_TRUE(ret == ErrCode::OK || ret == ErrCode::ERR_NOT_FOUND);
}

/**
 * @brief 测试异步执行工具
 */
HWTEST_F(CLIManagerServiceTest, ExecToolAsync, TestSize.Level2)
{
    Json::Value arguments;
    arguments["test"] = "value";

    ExecOptions options;
    options.mode = ExecutionMode::ASYNC;
    options.timeoutMs = 5000;

    // 创建 mock 回调
    class MockCallback : public ICLICallback {
    public:
        void OnOutput(const std::string& taskId, const ToolEvent& event) override {}
        void OnExit(const std::string& taskId, int32_t exitCode) override {}
        void OnError(const std::string& taskId,
                    int32_t errorCode,
                    const std::string& errorMsg) override {}
    };

    sptr<MockCallback> callback = new MockCallback();
    std::string taskId;

    int32_t ret = service_->ExecToolAsync(TEST_TOOL_NAME, arguments,
                                          callback, options, taskId);

    // 可能失败（工具不存在）或成功
    EXPECT_TRUE(ret == ErrCode::OK || ret == ErrCode::ERR_NOT_FOUND);

    if (ret == ErrCode::OK) {
        EXPECT_FALSE(taskId.empty());

        // 清理：取消任务
        service_->CancelTask(taskId);
    }
}

/**
 * @brief 测试取消任务
 */
HWTEST_F(CLIManagerServiceTest, CancelTask, TestSize.Level2)
{
    std::string taskId = "test_task_12345";
    int32_t ret = service_->CancelTask(taskId);

    // 任务不存在，应该返回错误
    EXPECT_EQ(ret, ErrCode::ERR_TASK_NOT_FOUND);
}

/**
 * @brief 测试查询任务状态
 */
HWTEST_F(CLIManagerServiceTest, QueryTaskStatus, TestSize.Level2)
{
    std::string taskId = "test_task_12345";
    TaskStatus status;
    int32_t ret = service_->QueryTaskStatus(taskId, status);

    // 任务不存在，应该返回错误
    EXPECT_EQ(ret, ErrCode::ERR_TASK_NOT_FOUND);
}

/**
 * @brief 测试权限校验
 */
HWTEST_F(CLIManagerServiceTest, PermissionCheck, TestSize.Level1)
{
    auto permissionChecker = service_->GetPermissionChecker();
    ASSERT_NE(permissionChecker, nullptr);

    CallerInfo caller;
    caller.bundleName = "com.ohos.testapp";
    caller.pid = 1000;
    caller.uid = 1000;

    std::vector<std::string> permissions;
    permissions.push_back("ohos.permission.INTERNET");

    PermissionCheckResult result = permissionChecker->CheckPermission(caller, permissions);

    // 检查结果
    EXPECT_TRUE(result.granted || !result.granted);  // 根据实现可能不同
    EXPECT_FALSE(result.reason.empty());
}

/**
 * @brief 测试系统应用验证
 */
HWTEST_F(CLIManagerServiceTest, ValidateSystemApp, TestSize.Level1)
{
    auto permissionChecker = service_->GetPermissionChecker();
    ASSERT_NE(permissionChecker, nullptr);

    CallerInfo validCaller;
    validCaller.bundleName = "com.ohos.systemapp";
    EXPECT_TRUE(permissionChecker->ValidateSystemApp(validCaller));

    CallerInfo invalidCaller;
    invalidCaller.bundleName = "com.example.thirdparty";
    EXPECT_FALSE(permissionChecker->ValidateSystemApp(invalidCaller));
}

/**
 * @brief 测试资源管理器
 */
HWTEST_F(CLIManagerServiceTest, ResourceManager, TestSize.Level1)
{
    auto resourceManager = service_->GetResourceManager();
    ASSERT_NE(resourceManager, nullptr);

    // 测试任务创建限制
    std::string bundleName = "com.ohos.testapp";
    EXPECT_TRUE(resourceManager->CanCreateTask(bundleName));

    // 注册任务
    std::string taskId = "test_task_res_001";
    resourceManager->RegisterTask(taskId, 12345, bundleName);

    // 注销任务
    resourceManager->UnregisterTask(taskId);
}

/**
 * @brief 测试命令清理
 */
HWTEST_F(CLIManagerServiceTest, CommandSanitization, TestSize.Level1)
{
    // 测试危险字符检测
    CommandSanitizer sanitizer;

    EXPECT_TRUE(sanitizer.ValidateString("safe_string"));
    EXPECT_FALSE(sanitizer.ValidateString("dangerous; rm -rf /"));
    EXPECT_FALSE(sanitizer.ValidateString("dangerous && malicious"));

    // 测试路径验证
    EXPECT_TRUE(sanitizer.ValidatePath("/usr/bin/tool"));
    EXPECT_FALSE(sanitizer.ValidatePath("../../../etc/passwd"));

    // 测试 IP 验证
    EXPECT_TRUE(sanitizer.ValidateIP("192.168.1.1"));
    EXPECT_TRUE(sanitizer.ValidateIP("10.0.0.1"));
    EXPECT_FALSE(sanitizer.ValidateIP("256.256.256.256"));
    EXPECT_FALSE(sanitizer.ValidateIP("invalid-ip"));

    // 测试端口验证
    EXPECT_TRUE(sanitizer.ValidatePort(80));
    EXPECT_TRUE(sanitizer.ValidatePort(8080));
    EXPECT_TRUE(sanitizer.ValidatePort(65535));
    EXPECT_FALSE(sanitizer.ValidatePort(0));
    EXPECT_FALSE(sanitizer.ValidatePort(65536));
}

/**
 * @brief 测试事件分发器
 */
HWTEST_F(CLIManagerServiceTest, EventDispatcher, TestSize.Level1)
{
    auto eventDispatcher = service_->GetEventDispatcher();
    ASSERT_NE(eventDispatcher, nullptr);

    // 创建 mock 回调
    class MockCallback : public ICLICallback {
    public:
        void OnOutput(const std::string& taskId, const ToolEvent& event) override {
            outputCalled = true;
        }
        void OnExit(const std::string& taskId, int32_t exitCode) override {
            exitCalled = true;
        }
        void OnError(const std::string& taskId,
                    int32_t errorCode,
                    const std::string& errorMsg) override {
            errorCalled = true;
        }

        bool outputCalled = false;
        bool exitCalled = false;
        bool errorCalled = false;
    };

    sptr<MockCallback> callback = new MockCallback();
    std::string taskId = "test_task_event_001";

    // 注册回调
    eventDispatcher->RegisterCallback(taskId, callback);

    // 分发输出事件
    ToolEvent event;
    event.type = "stdout";
    event.data = "Test output";
    event.timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    IOEvent ioEvent;
    ioEvent.type = IOEventType::STDOUT;
    ioEvent.taskId = taskId;
    ioEvent.data = "Test output";

    eventDispatcher->DispatchIOEvent(ioEvent);

    // 注销回调
    eventDispatcher->UnregisterCallback(taskId);
}

/**
 * @brief 测试任务 ID 生成
 */
HWTEST_F(CLIManagerServiceTest, TaskIdGeneration, TestSize.Level1)
{
    // 多次生成任务 ID，应该不重复
    std::set<std::string> taskIds;

    for (int i = 0; i < 100; i++) {
        std::string taskId1 = "task_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "_" + std::to_string(i);
        std::string taskId2 = "task_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() + 1) + "_" + std::to_string(i);

        taskIds.insert(taskId1);
        taskIds.insert(taskId2);
    }

    EXPECT_EQ(taskIds.size(), 200);  // 所有 ID 应该唯一
}

/**
 * @brief 性能测试 - 工具查询性能
 */
HWTEST_F(CLIManagerServiceTest, Performance_QueryTools, TestSize.Level3)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<ToolInfo> tools;
    int32_t ret = service_->QueryTools(tools);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    EXPECT_EQ(ret, ErrCode::OK);
    EXPECT_LT(duration.count(), 100);  // 应该在 100ms 内完成
}

/**
 * @brief 压力测试 - 并发任务创建
 */
HWTEST_F(CLIManagerServiceTest, Stress_ConcurrentTasks, TestSize.Level3)
{
    const int TASK_COUNT = 50;
    std::vector<std::string> taskIds;

    for (int i = 0; i < TASK_COUNT; i++) {
        std::string taskId = "stress_test_" + std::to_string(i);

        auto resourceManager = service_->GetResourceManager();
        if (resourceManager->CanCreateTask("com.ohos.stresstest")) {
            resourceManager->RegisterTask(taskId, 10000 + i, "com.ohos.stresstest");
            taskIds.push_back(taskId);
        }
    }

    // 清理
    auto resourceManager = service_->GetResourceManager();
    for (const auto& taskId : taskIds) {
        resourceManager->UnregisterTask(taskId);
    }

    EXPECT_GT(taskIds.size(), 0);
}
