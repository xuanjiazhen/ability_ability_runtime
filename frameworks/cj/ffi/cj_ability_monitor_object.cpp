/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "cj_ability_monitor_object.h"
#include "hilog_tag_wrapper.h"
#include <cstdint>

namespace {
    CJMonitorFuncs* g_cjMonitorFuncs = nullptr;
}

void RegisterCJMonitorFuncs(void (*registerFunc)(CJMonitorFuncs*))
{
  TAG_LOGD(AAFwkTag::DELEGATOR, "call");
  if (g_cjMonitorFuncs != nullptr) {
    TAG_LOGE(AAFwkTag::DELEGATOR, "repeated registration for cj functions");
    return;
  }

  if (registerFunc == nullptr) {
    TAG_LOGE(AAFwkTag::DELEGATOR, "null registerFunc");
  }

  g_cjMonitorFuncs = new CJMonitorFuncs();
  registerFunc(g_cjMonitorFuncs);
  TAG_LOGD(AAFwkTag::DELEGATOR, "end");
}

namespace OHOS {
namespace AbilityDelegatorCJ {

CJMonitorObject::CJMonitorObject(const int64_t monitorId)
    : monitorId_(monitorId) {}

void CJMonitorObject::OnAbilityCreate(const int64_t abilityId) {
  TAG_LOGD(AAFwkTag::DELEGATOR, "called");
  g_cjMonitorFuncs->cjOnAbilityCreate(monitorId_, abilityId);
}

void CJMonitorObject::OnAbilityForeground(const int64_t abilityId) {
  TAG_LOGD(AAFwkTag::DELEGATOR, "called");
  g_cjMonitorFuncs->cjOnAbilityForeground(monitorId_, abilityId);
}

void CJMonitorObject::OnAbilityBackground(const int64_t abilityId) {
  TAG_LOGD(AAFwkTag::DELEGATOR, "called");
  g_cjMonitorFuncs->cjOnAbilityBackground(monitorId_, abilityId);
}

void CJMonitorObject::OnAbilityDestroy(const int64_t abilityId) {
  TAG_LOGD(AAFwkTag::DELEGATOR, "called");
  g_cjMonitorFuncs->cjOnAbilityDestroy(monitorId_, abilityId);
}

void CJMonitorObject::OnWindowStageCreate(const int64_t abilityId) {
  TAG_LOGD(AAFwkTag::DELEGATOR, "called");
  g_cjMonitorFuncs->cjOnWindowStageCreate(monitorId_, abilityId);
}

void CJMonitorObject::OnWindowStageRestore(const int64_t abilityId) {
  TAG_LOGD(AAFwkTag::DELEGATOR, "called");
  g_cjMonitorFuncs->cjOnWindowStageRestore(monitorId_, abilityId);
}

void CJMonitorObject::OnWindowStageDestroy(const int64_t abilityId) {
  TAG_LOGD(AAFwkTag::DELEGATOR, "called");
  g_cjMonitorFuncs->cjOnWindowStageDestroy(monitorId_, abilityId);
}
} // namespace AbilityDelegatorJs
} // namespace OHOS
