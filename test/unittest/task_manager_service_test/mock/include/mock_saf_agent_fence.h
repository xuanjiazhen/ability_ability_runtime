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

#ifndef MOCK_SAF_AGENT_FENCE_H
#define MOCK_SAF_AGENT_FENCE_H

#include <cstdint>

namespace OHOS {
namespace Security {
namespace SAF {
// Test seam: controls the return value of the mocked SafAgentFence::RetrieveAuthTokenByTaskId.
void SetMockFenceResult(int32_t result);
}  // namespace SAF
}  // namespace Security
}  // namespace OHOS

#endif  // MOCK_SAF_AGENT_FENCE_H
