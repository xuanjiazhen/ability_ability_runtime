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

#include "multi_foreground_observer_adapter.h"

#ifdef SUPPORT_SCREEN
#include "window_focus_changed_listener.h"
#include "window_manager.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

bool MultiForegroundObserverAdapter::RegisterMultiForegroundFocusListener(
    const sptr<WindowFocusChangedListener>& listener)
{
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null focusListener");
        return false;
    }

    if (TryRegisterNewInterface(listener)) {
        usingFallback_ = false;
        newInterfaceAvailable_ = true;
        TAG_LOGI(AAFwkTag::APPMGR, "registered via new multi-foreground interface");
        return true;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "new interface unavailable, falling back to single-user monitoring");
    RegisterFallback(listener);
    usingFallback_ = true;
    return true;
}

void MultiForegroundObserverAdapter::UnregisterMultiForegroundFocusListener(
    const sptr<WindowFocusChangedListener>& listener)
{
    if (usingFallback_) {
        UnregisterFallback(listener);
    } else {
        // TODO: Unregister from the new multi-foreground interface once available
        // (window domain SceneSessionManager "listen to all foreground users" interface, OH 7.1)
        UnregisterFallback(listener);
    }
}

bool MultiForegroundObserverAdapter::IsNewInterfaceAvailable() const
{
    return newInterfaceAvailable_;
}

bool MultiForegroundObserverAdapter::TryRegisterNewInterface(
    const sptr<WindowFocusChangedListener>& listener)
{
    // TODO: When window domain's "listen to all foreground users" interface is
    // available (OH 7.1), call the new SceneSessionManager interface here to
    // register for all foreground users' app foreground/background events in one
    // registration. The new interface signature is TBD (proposal §7 open issue).
    //
    // Expected flow:
    // 1. Obtain SceneSessionManager proxy
    // 2. Call RegisterAllForegroundUsersFocusListener(listener) or equivalent
    // 3. Return true on success, false if interface missing or registration fails
    //
    // Currently returns false to trigger fallback path (placeholder/mock per ADR-6).
    TAG_LOGI(AAFwkTag::APPMGR, "new interface not yet available (placeholder)");
    return false;
}

void MultiForegroundObserverAdapter::RegisterFallback(
    const sptr<WindowFocusChangedListener>& listener)
{
    // Fallback: original single foreground user monitoring path via WindowManager
    WindowManager::GetInstance().RegisterFocusChangedListener(listener);
}

void MultiForegroundObserverAdapter::UnregisterFallback(
    const sptr<WindowFocusChangedListener>& listener)
{
    WindowManager::GetInstance().UnregisterFocusChangedListener(listener);
}

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // SUPPORT_SCREEN
