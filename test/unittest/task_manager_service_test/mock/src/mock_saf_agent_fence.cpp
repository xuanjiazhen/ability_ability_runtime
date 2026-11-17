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

#include "mock_saf_agent_fence.h"

#include <string>
#include <vector>

#include "saf_agent_fence.h"
#include "saf_result_code.h"

namespace OHOS {
namespace Security {
namespace SAF {
namespace {
int32_t g_mockFenceResult = SAF_SUCCESS;  // default: task valid
std::vector<int32_t> g_mockFenceToken;
}  // namespace

void SetMockFenceResult(int32_t result)
{
    g_mockFenceResult = result;
}

void SetMockFenceToken(const std::vector<int32_t> &token)
{
    g_mockFenceToken = token;
}

int32_t SafAgentFence::RetrieveAuthTokenByTaskId(int32_t callerToken, const std::string &taskId,
    std::vector<int32_t> &authToken)
{
    authToken = g_mockFenceToken;
    return g_mockFenceResult;
}
}  // namespace SAF
}  // namespace Security
}  // namespace OHOS
