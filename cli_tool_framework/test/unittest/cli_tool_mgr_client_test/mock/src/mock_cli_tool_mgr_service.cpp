/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mock_cli_tool_mgr_service.h"

#include "cli_error_code.h"
#include "function_info.h"
#include "mock_cli_tool_mgr_client_flag.h"

namespace OHOS {
namespace CliTool {
int32_t CliToolMgrClientFlag::retGetAllToolSummaries = ERR_OK;
int32_t CliToolMgrClientFlag::retGetToolInfoByName = ERR_OK;
int32_t CliToolMgrClientFlag::retGetAllToolInfos = ERR_OK;
int32_t CliToolMgrClientFlag::retRegisterFunction = ERR_OK;
int32_t CliToolMgrClientFlag::retBatchRegisterFunctions = ERR_OK;
int32_t CliToolMgrClientFlag::batchRegisterFunctionsSuccessCount = 0;
int32_t CliToolMgrClientFlag::retResetNamespaceFunctions = ERR_OK;
int32_t CliToolMgrClientFlag::resetNamespaceFunctionsSuccessCount = 0;
int32_t CliToolMgrClientFlag::retBatchRegisterFunctionsAsync = ERR_OK;
int32_t CliToolMgrClientFlag::retUnregisterIntentFunctionsByNamespaceAsync = ERR_OK;
int32_t CliToolMgrClientFlag::retResetNamespaceFunctionsAsync = ERR_OK;
int32_t CliToolMgrClientFlag::retGetFunctionInfo = ERR_OK;
int32_t CliToolMgrClientFlag::retUnregisterFunction = ERR_OK;
int32_t CliToolMgrClientFlag::retGetAllFunctions = ERR_OK;
int32_t CliToolMgrClientFlag::retGetPublicFunctions = ERR_OK;
int32_t CliToolMgrClientFlag::retGetPublicFunctionInfo = ERR_OK;
int32_t CliToolMgrClientFlag::retExecTool = ERR_OK;
int32_t CliToolMgrClientFlag::retExecCmd = ERR_OK;
int32_t CliToolMgrClientFlag::retSubscribeSession = ERR_OK;
int32_t CliToolMgrClientFlag::retUnsubscribeSession = ERR_OK;
int32_t CliToolMgrClientFlag::retClearSession = ERR_OK;
int32_t CliToolMgrClientFlag::retQuerySession = ERR_OK;
int32_t CliToolMgrClientFlag::retSendMessage = ERR_OK;
int32_t CliToolMgrClientFlag::retBatchQueryPermission = ERR_OK;
int32_t CliToolMgrClientFlag::retLoadSystemAbility = ERR_OK;
int32_t CliToolMgrClientFlag::retRegisterCliHook = ERR_OK;
int32_t CliToolMgrClientFlag::retUnregisterCliHook = ERR_OK;
int32_t CliToolMgrClientFlag::retRegisterFunctionHook = ERR_OK;
int32_t CliToolMgrClientFlag::retUnregisterFunctionHook = ERR_OK;
int32_t CliToolMgrClientFlag::retBeforeInvokeFunction = ERR_OK;
int32_t CliToolMgrClientFlag::retAfterInvokeFunction = ERR_OK;
int32_t CliToolMgrClientFlag::registerCliHookCount = 0;
int32_t CliToolMgrClientFlag::unregisterCliHookCount = 0;
int32_t CliToolMgrClientFlag::registerFunctionHookCount = 0;
int32_t CliToolMgrClientFlag::unregisterFunctionHookCount = 0;
int32_t CliToolMgrClientFlag::beforeInvokeFunctionCount = 0;
int32_t CliToolMgrClientFlag::afterInvokeFunctionCount = 0;
int32_t CliToolMgrClientFlag::lastHookActiveMethods = 0;
std::string CliToolMgrClientFlag::lastInvokeFunctionNamespace;
std::string CliToolMgrClientFlag::lastInvokeFunctionName;
bool CliToolMgrClientFlag::nullSystemAbility = false;
bool CliToolMgrClientFlag::shouldCallback = true;
bool CliToolMgrClientFlag::devModeEnabled = false;
sptr<IRemoteObject> CliToolMgrClientFlag::cliToolMgr = nullptr;
std::string CliToolMgrClientFlag::lastEventId;
std::string CliToolMgrClientFlag::lastSubscriptionId;
std::string CliToolMgrClientFlag::lastTaskId;
std::vector<ToolInfo> CliToolMgrClientFlag::toolInfos;
std::vector<FunctionInfo> CliToolMgrClientFlag::functionInfos;
std::vector<ToolSummary> CliToolMgrClientFlag::summaries;
CliSessionInfo CliToolMgrClientFlag::querySession;
std::vector<CommandPermission> CliToolMgrClientFlag::commandPermissions;

namespace {
void ResetCliToolRetCodes()
{
    CliToolMgrClientFlag::retGetAllToolSummaries = ERR_OK;
    CliToolMgrClientFlag::retGetToolInfoByName = ERR_OK;
    CliToolMgrClientFlag::retGetAllToolInfos = ERR_OK;
    CliToolMgrClientFlag::retRegisterFunction = ERR_OK;
    CliToolMgrClientFlag::retBatchRegisterFunctions = ERR_OK;
    CliToolMgrClientFlag::batchRegisterFunctionsSuccessCount = 0;
    CliToolMgrClientFlag::retResetNamespaceFunctions = ERR_OK;
    CliToolMgrClientFlag::resetNamespaceFunctionsSuccessCount = 0;
    CliToolMgrClientFlag::retBatchRegisterFunctionsAsync = ERR_OK;
    CliToolMgrClientFlag::retUnregisterIntentFunctionsByNamespaceAsync = ERR_OK;
    CliToolMgrClientFlag::retResetNamespaceFunctionsAsync = ERR_OK;
    CliToolMgrClientFlag::retGetFunctionInfo = ERR_OK;
    CliToolMgrClientFlag::retUnregisterFunction = ERR_OK;
    CliToolMgrClientFlag::retGetAllFunctions = ERR_OK;
    CliToolMgrClientFlag::retGetPublicFunctions = ERR_OK;
    CliToolMgrClientFlag::retGetPublicFunctionInfo = ERR_OK;
    CliToolMgrClientFlag::retExecTool = ERR_OK;
    CliToolMgrClientFlag::retExecCmd = ERR_OK;
    CliToolMgrClientFlag::retSubscribeSession = ERR_OK;
    CliToolMgrClientFlag::retUnsubscribeSession = ERR_OK;
    CliToolMgrClientFlag::retClearSession = ERR_OK;
    CliToolMgrClientFlag::retQuerySession = ERR_OK;
    CliToolMgrClientFlag::retSendMessage = ERR_OK;
    CliToolMgrClientFlag::retBatchQueryPermission = ERR_OK;
    CliToolMgrClientFlag::retLoadSystemAbility = ERR_OK;
    CliToolMgrClientFlag::retRegisterCliHook = ERR_OK;
    CliToolMgrClientFlag::retUnregisterCliHook = ERR_OK;
    CliToolMgrClientFlag::retRegisterFunctionHook = ERR_OK;
    CliToolMgrClientFlag::retUnregisterFunctionHook = ERR_OK;
    CliToolMgrClientFlag::retBeforeInvokeFunction = ERR_OK;
    CliToolMgrClientFlag::retAfterInvokeFunction = ERR_OK;
    CliToolMgrClientFlag::registerCliHookCount = 0;
    CliToolMgrClientFlag::unregisterCliHookCount = 0;
    CliToolMgrClientFlag::registerFunctionHookCount = 0;
    CliToolMgrClientFlag::unregisterFunctionHookCount = 0;
    CliToolMgrClientFlag::beforeInvokeFunctionCount = 0;
    CliToolMgrClientFlag::afterInvokeFunctionCount = 0;
    CliToolMgrClientFlag::lastHookActiveMethods = 0;
}

void ResetCliToolRuntimeState()
{
    CliToolMgrClientFlag::lastInvokeFunctionNamespace.clear();
    CliToolMgrClientFlag::lastInvokeFunctionName.clear();
    CliToolMgrClientFlag::nullSystemAbility = false;
    CliToolMgrClientFlag::shouldCallback = true;
    CliToolMgrClientFlag::devModeEnabled = false;
    CliToolMgrClientFlag::cliToolMgr = nullptr;
    CliToolMgrClientFlag::lastEventId.clear();
    CliToolMgrClientFlag::lastSubscriptionId.clear();
    CliToolMgrClientFlag::lastTaskId.clear();
    CliToolMgrClientFlag::toolInfos.clear();
    CliToolMgrClientFlag::functionInfos.clear();
    CliToolMgrClientFlag::summaries.clear();
    CliToolMgrClientFlag::querySession = {};
    CliToolMgrClientFlag::commandPermissions.clear();
}
} // namespace

void CliToolMgrClientFlag::Reset()
{
    ResetCliToolRetCodes();
    ResetCliToolRuntimeState();
}

int32_t MockCliToolMgrService::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    summaries = CliToolMgrClientFlag::summaries;
    return CliToolMgrClientFlag::retGetAllToolSummaries;
}

int32_t MockCliToolMgrService::GetToolInfoByName(const std::string &, ToolInfo &tool)
{
    if (!CliToolMgrClientFlag::toolInfos.empty()) {
        tool = CliToolMgrClientFlag::toolInfos.front();
    }
    return CliToolMgrClientFlag::retGetToolInfoByName;
}

int32_t MockCliToolMgrService::GetAllToolInfos(ToolsRawData &tools)
{
    if (CliToolMgrClientFlag::retGetAllToolInfos == ERR_OK) {
        ToolsRawData::FromToolInfoVec(CliToolMgrClientFlag::toolInfos, tools);
    }
    return CliToolMgrClientFlag::retGetAllToolInfos;
}

int32_t MockCliToolMgrService::RegisterFunction(const FunctionInfo &function)
{
    CliToolMgrClientFlag::functionInfos.push_back(function);
    return CliToolMgrClientFlag::retRegisterFunction;
}

int32_t MockCliToolMgrService::BatchRegisterFunctions(const FunctionsRawData &functions,
    int32_t &successCount)
{
    std::vector<FunctionInfo> functionList;
    FunctionsRawData::ToFunctionInfoVec(functions, functionList);
    for (const auto &function : functionList) {
        CliToolMgrClientFlag::functionInfos.push_back(function);
    }
    successCount = CliToolMgrClientFlag::batchRegisterFunctionsSuccessCount > 0 ?
        CliToolMgrClientFlag::batchRegisterFunctionsSuccessCount :
        static_cast<int32_t>(functionList.size());
    return CliToolMgrClientFlag::retBatchRegisterFunctions;
}

int32_t MockCliToolMgrService::GetFunctionInfo(const std::string &, const std::string &,
    FunctionInfo &function)
{
    if (!CliToolMgrClientFlag::functionInfos.empty()) {
        function = CliToolMgrClientFlag::functionInfos.front();
    }
    return CliToolMgrClientFlag::retGetFunctionInfo;
}

int32_t MockCliToolMgrService::UnregisterFunction(int32_t, const std::string &, const std::string &)
{
    if (!CliToolMgrClientFlag::functionInfos.empty()) {
        CliToolMgrClientFlag::functionInfos.erase(CliToolMgrClientFlag::functionInfos.begin());
    }
    return CliToolMgrClientFlag::retUnregisterFunction;
}

int32_t MockCliToolMgrService::UnregisterIntentFunctionsByNamespace(int32_t, const std::string &)
{
    return CliToolMgrClientFlag::retUnregisterFunction;
}

int32_t MockCliToolMgrService::ResetNamespaceFunctions(int32_t, const std::string &,
    const FunctionsRawData &functions, int32_t &successCount)
{
    std::vector<FunctionInfo> functionList;
    FunctionsRawData::ToFunctionInfoVec(functions, functionList);
    for (const auto &function : functionList) {
        CliToolMgrClientFlag::functionInfos.push_back(function);
    }
    successCount = CliToolMgrClientFlag::resetNamespaceFunctionsSuccessCount > 0 ?
        CliToolMgrClientFlag::resetNamespaceFunctionsSuccessCount :
        static_cast<int32_t>(functionList.size());
    return CliToolMgrClientFlag::retResetNamespaceFunctions;
}

int32_t MockCliToolMgrService::GetAllFunctions(FunctionsRawData &functions)
{
    FunctionsRawData::FromFunctionInfoVec(CliToolMgrClientFlag::functionInfos, functions);
    return CliToolMgrClientFlag::retGetAllFunctions;
}

int32_t MockCliToolMgrService::GetPublicFunctions(FunctionsRawData &functions)
{
    FunctionsRawData::FromFunctionInfoVec(CliToolMgrClientFlag::functionInfos, functions);
    return CliToolMgrClientFlag::retGetPublicFunctions;
}

int32_t MockCliToolMgrService::GetPublicFunctionInfo(const std::string &,
    const std::string &, FunctionInfo &function)
{
    if (!CliToolMgrClientFlag::functionInfos.empty()) {
        function = CliToolMgrClientFlag::functionInfos.front();
    }
    return CliToolMgrClientFlag::retGetPublicFunctionInfo;
}

int32_t MockCliToolMgrService::BatchRegisterFunctionsAsync(const FunctionsRawData &functions)
{
    std::vector<FunctionInfo> functionList;
    FunctionsRawData::ToFunctionInfoVec(functions, functionList);
    for (const auto &function : functionList) {
        CliToolMgrClientFlag::functionInfos.push_back(function);
    }
    return CliToolMgrClientFlag::retBatchRegisterFunctions;
}

int32_t MockCliToolMgrService::UnregisterIntentFunctionsByNamespaceAsync(int32_t, const std::string &)
{
    return CliToolMgrClientFlag::retUnregisterFunction;
}

int32_t MockCliToolMgrService::ResetNamespaceFunctionsAsync(int32_t, const std::string &,
    const FunctionsRawData &functions)
{
    std::vector<FunctionInfo> functionList;
    FunctionsRawData::ToFunctionInfoVec(functions, functionList);
    for (const auto &function : functionList) {
        CliToolMgrClientFlag::functionInfos.push_back(function);
    }
    return CliToolMgrClientFlag::retResetNamespaceFunctions;
}

int32_t MockCliToolMgrService::ExecTool(const ExecToolParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &)
{
    CliToolMgrClientFlag::lastEventId = eventId;
    CliToolMgrClientFlag::lastTaskId = param.options.taskId;
    return CliToolMgrClientFlag::retExecTool;
}

int32_t MockCliToolMgrService::ExecCmd(const ExecCmdParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &, const std::string &)
{
    CliToolMgrClientFlag::lastEventId = eventId;
    CliToolMgrClientFlag::lastTaskId = param.execCmdOptions.taskId;
    return CliToolMgrClientFlag::retExecCmd;
}

int32_t MockCliToolMgrService::SubscribeSession(const std::string &, const std::string &subscriptionId,
    const sptr<ICliToolManagerScheduler> &)
{
    CliToolMgrClientFlag::lastSubscriptionId = subscriptionId;
    return CliToolMgrClientFlag::retSubscribeSession;
}

int32_t MockCliToolMgrService::UnsubscribeSession(const std::string &, const std::string &)
{
    return CliToolMgrClientFlag::retUnsubscribeSession;
}

int32_t MockCliToolMgrService::ClearSession(const std::string &)
{
    return CliToolMgrClientFlag::retClearSession;
}

int32_t MockCliToolMgrService::QuerySession(const std::string &, CliSessionInfo &session)
{
    session = CliToolMgrClientFlag::querySession;
    return CliToolMgrClientFlag::retQuerySession;
}

int32_t MockCliToolMgrService::SendMessage(const std::string &, const std::string &, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &)
{
    CliToolMgrClientFlag::lastEventId = eventId;
    return CliToolMgrClientFlag::retSendMessage;
}

int32_t MockCliToolMgrService::BatchQueryPermissionBySubCommand(
    const std::vector<Command> &, std::vector<CommandPermission> &cmdPermissions)
{
    cmdPermissions = CliToolMgrClientFlag::commandPermissions;
    return CliToolMgrClientFlag::retBatchQueryPermission;
}

int32_t MockCliToolMgrService::RegisterCliHook(const sptr<ICliHookInterface> &, int32_t activeMethods)
{
    CliToolMgrClientFlag::registerCliHookCount++;
    CliToolMgrClientFlag::lastHookActiveMethods = activeMethods;
    return CliToolMgrClientFlag::retRegisterCliHook;
}

int32_t MockCliToolMgrService::UnregisterCliHook(const sptr<ICliHookInterface> &)
{
    CliToolMgrClientFlag::unregisterCliHookCount++;
    return CliToolMgrClientFlag::retUnregisterCliHook;
}

int32_t MockCliToolMgrService::RegisterFunctionHook(const sptr<IFunctionHookInterface> &, int32_t activeMethods)
{
    CliToolMgrClientFlag::registerFunctionHookCount++;
    CliToolMgrClientFlag::lastHookActiveMethods = activeMethods;
    return CliToolMgrClientFlag::retRegisterFunctionHook;
}

int32_t MockCliToolMgrService::UnregisterFunctionHook(const sptr<IFunctionHookInterface> &)
{
    CliToolMgrClientFlag::unregisterFunctionHookCount++;
    return CliToolMgrClientFlag::retUnregisterFunctionHook;
}

int32_t MockCliToolMgrService::BeforeInvokeFunction(InvokeFunctionParam &param)
{
    CliToolMgrClientFlag::beforeInvokeFunctionCount++;
    CliToolMgrClientFlag::lastInvokeFunctionNamespace = param.functionNamespace;
    CliToolMgrClientFlag::lastInvokeFunctionName = param.functionName;
    return CliToolMgrClientFlag::retBeforeInvokeFunction;
}

int32_t MockCliToolMgrService::AfterInvokeFunction(FunctionResultWrap &)
{
    CliToolMgrClientFlag::afterInvokeFunctionCount++;
    return CliToolMgrClientFlag::retAfterInvokeFunction;
}
} // namespace CliTool
} // namespace OHOS
