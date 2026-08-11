/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "interceptor/block_all_app_start_interceptor.h"

#include "ability_manager_service.h"
#include "ability_record.h"
#include "hilog_tag_wrapper.h"
#include "errors.h"
#include "ability_config.h"

namespace OHOS {
namespace AAFwk {
void BlockAllAppStartInterceptor::SetShouldBlockFunc(const std::function<bool()>& func)
{
    shouldBlockFunc_ = func;
}

void BlockAllAppStartInterceptor::SetIsAbilityStartedFunc(const IsAbilityStartedFunc& func)
{
    isAbilityStartedFunc_ = func;
}

void BlockAllAppStartInterceptor::SetAllowStartFunc(const AllowStartFunc& func)
{
    allowStartFunc_ = func;
}

void BlockAllAppStartInterceptor::SetStrictBlockFunc(const std::function<bool()>& func)
{
    strictBlockFunc_ = func;
}

ErrCode BlockAllAppStartInterceptor::Execute()
{
    if (!shouldBlockFunc_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "shouldBlockFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!shouldBlockFunc_()) {
        return ERR_OK;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "blocking app start by current system policy");
    return ERR_ALL_APP_START_BLOCKED;
}

ErrCode BlockAllAppStartInterceptor::Execute(AbilityRequest& abilityRequest, int32_t validUserId)
{
    if (!shouldBlockFunc_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "shouldBlockFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!isAbilityStartedFunc_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "isAbilityStartedFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!shouldBlockFunc_()) {
        return ERR_OK;
    }

    // 恢复模式复用现有全局启动拦截器，白名单应用和受控备份扩展在拦截前直接放行。
    const auto &bundleName = abilityRequest.abilityInfo.bundleName;
    if (allowStartFunc_ && allowStartFunc_(abilityRequest)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "allow app in current system policy, bundle=%{public}s", bundleName.c_str());
        return ERR_OK;
    }

    // 恢复模式只允许白名单应用，不能沿用低内存场景“已启动页面可继续拉起”的例外。
    if (strictBlockFunc_ && strictBlockFunc_()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "blocking non-essential app in system recovery mode");
        return ERR_ALL_APP_START_BLOCKED;
    }

    if (abilityRequest.abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        if (isAbilityStartedFunc_(abilityRequest, validUserId)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "UIAbility already started, allowing start despite block mode");
            return ERR_OK;
        }
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "blocking app start by current system policy");
    return ERR_ALL_APP_START_BLOCKED;
}

ErrCode BlockAllAppStartInterceptor::DoProcess(const AbilityInterceptorParam &param)
{
    // DoProcess 执行时目标 Ability 可能尚未解析，不能在这里根据 Want 参数放行 BackupExtension；
    // 受控备份例外只在 Execute(AbilityRequest&) 中对已解析的真实目标进行判断。
    if (param.shouldBlockAllAppStartFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "shouldBlockAllAppStartFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (param.shouldBlockAllAppStartFunc_()) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "blocking app start by current system policy");
        return ERR_ALL_APP_START_BLOCKED;
    }
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
