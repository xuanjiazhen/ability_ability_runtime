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

#ifndef OHOS_ABILITY_RUNTIME_TASK_ATTR_PARAM_KEY_H
#define OHOS_ABILITY_RUNTIME_TASK_ATTR_PARAM_KEY_H

namespace OHOS {
namespace AAFwk {
// System-reserved Want parameter key carrying the taskId across a start-family call;
// single source shared by the AMS entry filter (ams_want_param_filter), the abilitymgr
// lifecycle report path and the task manager. Read-only: never stripped or consumed by
// removal. Caller-originated data: visibility to the target app is accepted by design —
// trust sits in the ownership + liveness check at consumption, not key secrecy.
constexpr const char *TASK_ATTRIBUTION_PARAM_KEY = "ohos.task.id";
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_TASK_ATTR_PARAM_KEY_H
