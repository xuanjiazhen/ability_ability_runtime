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

#include "ets_startup_manager.h"
#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"
#include "napi/native_api.h"
#include "startup_manager.h"
#include "sts_error_utils.h"
#include <memory>


namespace OHOS {
namespace AbilityRuntime {

int32_t EtsStartupManager::GetConfig(ani_env *env, ani_int value, std::shared_ptr<StartupConfig> &config)
{
  TAG_LOGD(AAFwkTag::STARTUP, "called");

  if (env == nullptr) {
    TAG_LOGE(AAFwkTag::STARTUP, "null env");
    return ERR_STARTUP_INVALID_VALUE;
  }
}

void EtsStartupManager::OnConfig(std::shared_ptr<StartupConfig> &config)
{
  config_ = config;
}

} // namespace AbilityRuntime
} // namespace OHOS