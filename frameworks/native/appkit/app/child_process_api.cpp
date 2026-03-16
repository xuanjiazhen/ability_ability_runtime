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
 
#include "child_process_api.h"
#include "child_thread_bridge.h"
#include "hilog_tag_wrapper.h"
 
namespace OHOS {
namespace AppExecFwk {
 
void ChildProcessApi::StartChild(const std::map<std::string, int32_t> &fds)
{
    TAG_LOGI(AAFwkTag::APPKIT, "ChildProcessApi StartChild, fds size:%{public}zu", fds.size());
    ChildThreadBridge::StartChild(fds);
}
 
}  // namespace AppExecFwk
}  // namespace OHOS