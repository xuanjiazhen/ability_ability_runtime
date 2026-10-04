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

#ifndef OHOS_ABILITY_RUNTIME_MULTI_FOREGROUND_OBSERVER_ADAPTER_H
#define OHOS_ABILITY_RUNTIME_MULTI_FOREGROUND_OBSERVER_ADAPTER_H

#include "refbase.h"

#ifdef SUPPORT_SCREEN
namespace OHOS {
namespace AppExecFwk {

class WindowFocusChangedListener;

/**
 * @brief Multi-foreground observer adapter.
 *
 * This adapter isolates the window domain's "listen to all foreground users" new
 * interface (OH 7.1, cross-module external dependency). When the new interface is
 * available, RegisterMultiForegroundFocusListener will register with it to listen
 * to all foreground users' app foreground/background events in one registration.
 * When the new interface is unavailable, it falls back to the original single
 * foreground user monitoring path via WindowManager.
 *
 * Design refs: ADR-1, ADR-2, ADR-6 in design.md.
 * Spec refs: R-1 (AC-1.1), R-2 (AC-1.2).
 */
class MultiForegroundObserverAdapter {
public:
    MultiForegroundObserverAdapter() = default;
    ~MultiForegroundObserverAdapter() = default;

    /**
     * @brief Register focus changed listener for all foreground users.
     *
     * Attempts to register via the window domain's new "listen to all foreground
     * users" interface. If the new interface is unavailable (missing or
     * registration fails), falls back to the original single foreground user
     * monitoring path via WindowManager.
     *
     * @param focusListener The focus changed listener to register.
     * @return true if registration succeeded (new interface or fallback);
     *         false if both paths failed.
     */
    bool RegisterMultiForegroundFocusListener(
        const sptr<WindowFocusChangedListener>& focusListener);

    /**
     * @brief Unregister focus changed listener.
     *
     * Unregisters from whichever registration path was used (new interface or
     * fallback).
     *
     * @param focusListener The focus changed listener to unregister.
     */
    void UnregisterMultiForegroundFocusListener(
        const sptr<WindowFocusChangedListener>& focusListener);

    /**
     * @brief Check whether the new multi-foreground interface is available.
     *
     * @return true if the new interface is available; false otherwise.
     */
    bool IsNewInterfaceAvailable() const;

private:
    bool TryRegisterNewInterface(
        const sptr<WindowFocusChangedListener>& focusListener);
    void RegisterFallback(
        const sptr<WindowFocusChangedListener>& focusListener);
    void UnregisterFallback(
        const sptr<WindowFocusChangedListener>& focusListener);

    bool usingFallback_ = true;
    bool newInterfaceAvailable_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // SUPPORT_SCREEN
#endif  // OHOS_ABILITY_RUNTIME_MULTI_FOREGROUND_OBSERVER_ADAPTER_H
