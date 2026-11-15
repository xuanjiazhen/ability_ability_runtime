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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCELLATION_SIGNAL_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCELLATION_SIGNAL_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class InsightIntentCancellationSignal
 * Observability surface exposed to the target app (JS/ETS/Cangjie) for an
 * in-flight insight intent execution. The signal is the single shared object
 * between the running executor and the InsightIntentContext handed to the app
 * code. When AMS cancels the intent, the executor calls
 * {@link InsightIntentExecutor::Cancel} (on the reverse IPC thread), which in
 * turn calls {@link Cancel} here to flip the flag and fire any registered
 * callback.
 *
 * Thread-safety: {@link Cancel} may run on the IPC thread while the app code
 * reads {@link IsCanceled} on its own thread, so the flag is atomic and the
 * callback registration is guarded by a mutex. A subscriber registered after
 * the signal is already cancelled MUST check {@link IsCanceled} right after
 * registration, because a late registration will never be invoked.
 */
class InsightIntentCancellationSignal {
public:
    using CancelCallback = std::function<void(int32_t reason)>;

    InsightIntentCancellationSignal() = default;
    ~InsightIntentCancellationSignal()
    {
        // Break the callback while holding no reference: the callback may own
        // language-level references (e.g. napi_ref) that must be released before
        // the runtime the signal belongs to is torn down.
        SetOnCancel(nullptr);
    }

    /**
     * @brief Mark the signal as cancelled and fire the registered callback once.
     *        The first cancellation wins; later calls are ignored.
     *
     * @param reason The cancellation reason, see AAFwk::InsightIntentCancelReason.
     */
    void Cancel(int32_t reason)
    {
        CancelCallback callback;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (canceled_.load(std::memory_order_acquire)) {
                return;
            }
            canceled_.store(true, std::memory_order_release);
            reason_ = reason;
            callback = onCancel_;
        }
        if (callback) {
            callback(reason);
        }
    }

    /**
     * @brief Check whether the execution has been cancelled.
     */
    bool IsCanceled() const
    {
        return canceled_.load(std::memory_order_acquire);
    }

    /**
     * @brief Get the cancellation reason, meaningful only after IsCanceled.
     */
    int32_t GetReason() const
    {
        return reason_;
    }

    /**
     * @brief Register (or clear, with nullptr) the cancellation callback.
     *
     * If the signal is already cancelled at registration time, the callback is
     * invoked immediately so a late registration never silently misses the
     * cancellation.
     */
    void SetOnCancel(CancelCallback callback)
    {
        bool alreadyCanceled = false;
        int32_t reason = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            onCancel_ = std::move(callback);
            alreadyCanceled = canceled_.load(std::memory_order_acquire);
            if (alreadyCanceled) {
                reason = reason_;
                callback = onCancel_;
            }
        }
        if (alreadyCanceled && callback) {
            callback(reason);
        }
    }

private:
    std::atomic<bool> canceled_ { false };
    int32_t reason_ = 0;
    mutable std::mutex mutex_;
    CancelCallback onCancel_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCELLATION_SIGNAL_H