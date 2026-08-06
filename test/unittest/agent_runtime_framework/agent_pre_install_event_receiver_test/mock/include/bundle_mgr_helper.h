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

#ifndef MOCK_BUNDLE_MGR_HELPER_H
#define MOCK_BUNDLE_MGR_HELPER_H

// Minimal stub: agent_card_mgr.h only needs BundleMgrHelper to be a complete type so the header
// parses. The receiver-under-test never instantiates or calls BundleMgrHelper (AgentCardMgr itself
// is mocked by mock_agent_card_mgr.cpp), so no members are required.
namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelper {};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_BUNDLE_MGR_HELPER_H
