/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#define protected public
#define private public
#include "cli_tool_manager_service.h"
#undef private
#undef protected

#include "bool_wrapper.h"
#include "cli_error_code.h"
#include "cli_mgr_service_mock.h"
#include "string_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
class CliToolManagerServiceMockTest : public testing::Test {
public:
    void SetUp() override
    {
        CliMgrServiceMock::Reset();
        service_ = CliToolManagerService::GetInstance();
        service_->ioMonitor_ = IOMonitor::Create();
        service_->interfaceCalledCount_.store(0);
        service_->sessionRecords_.clear();
        service_->bundleObservers_.clear();
    }

    void TearDown() override
    {
        service_->sessionRecords_.clear();
        service_->bundleObservers_.clear();
        CliMgrServiceMock::Reset();
    }

    sptr<CliToolManagerService> service_;
};

/**
 * @tc.name: SetupAndStartSession_0100
 * @tc.desc: Test child process creation failure branch
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, SetupAndStartSession_0100, TestSize.Level1)
{
    ExecToolParam param;
    param.toolName = "mock_tool";
    ToolInfo toolInfo;
    CliMgrServiceMock::createChildProcessResult = ERR_INVALID_PARAM;

    EXPECT_EQ(service_->SetupAndStartSession(param, "event", toolInfo, "{}", "bundle"), ERR_INVALID_PARAM);
    EXPECT_TRUE(service_->sessionRecords_.empty());
}

/**
 * @tc.name: SetupAndStartSession_0200
 * @tc.desc: Test monitor registration failure removes created session
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, SetupAndStartSession_0200, TestSize.Level1)
{
    ExecToolParam param;
    param.toolName = "mock_tool";
    ToolInfo toolInfo;
    CliMgrServiceMock::registerSessionResult = ERR_NO_INIT;

    EXPECT_EQ(service_->SetupAndStartSession(param, "event", toolInfo, "{}", "bundle"), ERR_NO_INIT);
    EXPECT_TRUE(service_->sessionRecords_.empty());
}

/**
 * @tc.name: SetupAndStartSession_0300
 * @tc.desc: Test successful background session path
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, SetupAndStartSession_0300, TestSize.Level1)
{
    ExecToolParam param;
    param.toolName = "mock_tool";
    param.options.background = true;
    ToolInfo toolInfo;

    EXPECT_EQ(service_->SetupAndStartSession(param, "event", toolInfo, "{}", "bundle.ok"), ERR_OK);
    EXPECT_EQ(service_->sessionRecords_.size(), 1u);
    EXPECT_EQ(service_->bundleObservers_.size(), 1u);
}

/**
 * @tc.name: ValidateAndPrepareTool_0100
 * @tc.desc: Test tool lookup failure branch
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, ValidateAndPrepareTool_0100, TestSize.Level1)
{
    ExecToolParam param;
    param.toolName = "missing_tool";
    ToolInfo toolInfo;
    std::string sandboxConfig;
    std::string bundleName;
    CliMgrServiceMock::getToolByNameResult = ERR_TOOL_NOT_EXIST;

    EXPECT_EQ(service_->ValidateAndPrepareTool(param, 0, toolInfo, sandboxConfig, bundleName), ERR_TOOL_NOT_EXIST);
}

/**
 * @tc.name: RegisterAppStateObserver_0100
 * @tc.desc: Test observer connect failure, register failure, success and duplicate branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, RegisterAppStateObserver_0100, TestSize.Level1)
{
    CliMgrServiceMock::connectAppMgrResult = AppExecFwk::AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    service_->RegisterAppStateObserver("bundle.connect.fail", 100);
    EXPECT_TRUE(service_->bundleObservers_.empty());

    CliMgrServiceMock::connectAppMgrResult = AppExecFwk::AppMgrResultCode::RESULT_OK;
    CliMgrServiceMock::registerAppObserverResult = ERR_INVALID_PARAM;
    service_->RegisterAppStateObserver("bundle.register.fail", 100);
    EXPECT_TRUE(service_->bundleObservers_.empty());

    CliMgrServiceMock::registerAppObserverResult = ERR_OK;
    service_->RegisterAppStateObserver("bundle.ok", 100);
    EXPECT_EQ(service_->bundleObservers_.size(), 1u);
    service_->RegisterAppStateObserver("bundle.ok", 100);
    EXPECT_EQ(service_->bundleObservers_.size(), 1u);
}

/**
 * @tc.name: ValidateSkillTypeFromParam_0100
 * @tc.desc: Test missing skillName and successful normalized skill query branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, ValidateSkillTypeFromParam_0100, TestSize.Level1)
{
    ExecToolParam param;
    int32_t skillType = 0;
    EXPECT_EQ(service_->ValidateSkillTypeFromParam(param, skillType), ERR_INVALID_VALUE);

    param.args.SetParam("--skillName", AAFwk::String::Box("skill"));
    param.args.SetParam("--bundleName", AAFwk::String::Box("bundle"));
    param.args.SetParam("--moduleName", AAFwk::String::Box("module"));
    CliMgrServiceMock::skillType = 2;
    EXPECT_EQ(service_->ValidateSkillTypeFromParam(param, skillType), ERR_OK);
    EXPECT_EQ(skillType, 2);
    EXPECT_EQ(CliMgrServiceMock::lastSkillName, "skill");
}

/**
 * @tc.name: SetupAndStartSkillSession_0100
 * @tc.desc: Test skill execute failure and background success branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, SetupAndStartSkillSession_0100, TestSize.Level1)
{
    ExecToolParam param;
    param.toolName = "ohos-arkTSScript";
    param.options.background = true;
    param.args.SetParam("skillName", AAFwk::String::Box("skill"));
    param.args.SetParam("args", AAFwk::String::Box(R"({"target":"device"})"));
    ToolInfo toolInfo;

    CliMgrServiceMock::executeSkillResult = ERR_INVALID_PARAM;
    EXPECT_EQ(service_->SetupAndStartSkillSession(param, "event", toolInfo), ERR_INVALID_PARAM);
    EXPECT_TRUE(service_->sessionRecords_.empty());

    CliMgrServiceMock::executeSkillResult = ERR_OK;
    EXPECT_EQ(service_->SetupAndStartSkillSession(param, "event", toolInfo), ERR_OK);
    EXPECT_EQ(service_->sessionRecords_.size(), 1u);
}

/**
 * @tc.name: OnStop_0100
 * @tc.desc: Test stop clears sessions, kills active pids and stops io monitor
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceMockTest, OnStop_0100, TestSize.Level1)
{
    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "stop_session";
    record->processId = 1001;
    record->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(record);

    service_->OnStop();
    EXPECT_TRUE(service_->sessionRecords_.empty());
    EXPECT_EQ(CliMgrServiceMock::stopCount, 1);
}
} // namespace CliTool
} // namespace OHOS
