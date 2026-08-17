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

#ifndef OHOS_ABILITY_RUNTIME_BLOCK_ALL_APP_START_INTERCEPTOR
#define OHOS_ABILITY_RUNTIME_BLOCK_ALL_APP_START_INTERCEPTOR

#include <functional>
#include <memory>
#include "ability_interceptor_interface.h"

namespace OHOS {
namespace AAFwk {
struct AbilityRequest;
class BlockAllAppStartInterceptor : public IAbilityInterceptor {
public:
    BlockAllAppStartInterceptor() = default;
    ~BlockAllAppStartInterceptor() = default;
    ErrCode DoProcess(const AbilityInterceptorParam &param) override;

    void SetShouldBlockFunc(const std::function<bool()>& func);

    using IsAbilityStartedFunc = std::function<bool(AbilityRequest&, int32_t)>;
    void SetIsAbilityStartedFunc(const IsAbilityStartedFunc& func);

    // 根据完整的启动请求判断是否允许恢复模式例外启动。
    // 仅传 Bundle 名称无法区分普通 Ability 和 BackupExtensionAbility。
    // 回调可以在 AMS 完成认证后给当前请求附加一次性内部启动上下文。
    using AllowStartFunc = std::function<bool(AbilityRequest&)>;
    void SetAllowStartFunc(const AllowStartFunc& func);

    // 严格模式禁止沿用低内存场景“已启动页面继续运行”的历史例外。
    void SetStrictBlockFunc(const std::function<bool()>& func);

    ErrCode Execute();
    ErrCode Execute(AbilityRequest& abilityRequest, int32_t validUserId);
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };

private:
    std::function<bool()> shouldBlockFunc_;
    IsAbilityStartedFunc isAbilityStartedFunc_;
    // 保存由 AMS 注入的恢复白名单和受控备份判断。
    AllowStartFunc allowStartFunc_;
    // 标识当前是否采用系统恢复模式的严格阻断语义。
    std::function<bool()> strictBlockFunc_;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_BLOCK_ALL_APP_START_INTERCEPTOR
