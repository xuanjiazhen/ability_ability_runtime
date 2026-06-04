/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_H

#include <cstdint>
#include <string>
#include <vector>

#include "tool_info.h"

namespace OHOS {
namespace CliTool {
class CliToolDataManagerMock {
public:
    static int32_t getToolByNameResult;
    static int32_t getAllToolsResult;
    static bool toolIsPublic;
    static bool toolHasSubCommand;
    static std::string subCommandName;
    static std::vector<std::string> toolPermissions;
    static std::vector<std::string> subCommandPermissions;
    static std::vector<ToolInfo> allTools;

    static void Reset();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_H
