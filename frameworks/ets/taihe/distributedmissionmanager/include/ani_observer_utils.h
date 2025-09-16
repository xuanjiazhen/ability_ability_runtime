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
#ifndef OHOS_ANI_OBSERVER_UTILS_H
#define OHOS_ANI_OBSERVER_UTILS_H
#include <string>
#include <optional>

#include "taihe/runtime.hpp"
#include "ohos.distributedmissionmanager.proj.hpp"
#include "ohos.distributedmissionmanager.impl.hpp"
#include "event_handler.h"
#include "event_runner.h"
#include "remote_on_listener_stub.h"

namespace ani_observerutils {

using namespace OHOS;

class GlobalRefGuard {
    ani_env *env_ = nullptr;
    ani_ref ref_ = nullptr;

public:
    GlobalRefGuard(ani_env *env, ani_object obj) : env_(env)
    {
        if (!env_ || !obj)
            return;
        if (ANI_OK != env_->GlobalReference_Create(obj, &ref_)) {
            ref_ = nullptr;
        }
    }
    explicit operator bool() const
    {
        return ref_ != nullptr;
    }
    ani_ref get() const
    {
        return ref_;
    }
    ~GlobalRefGuard()
    {
        if (env_ && ref_) {
            env_->GlobalReference_Delete(ref_);
        }
    }

    GlobalRefGuard(const GlobalRefGuard &) = delete;
    GlobalRefGuard &operator=(const GlobalRefGuard &) = delete;
};

using JsOnCallbackType = ::taihe::callback<void(::ohos::distributedmissionmanager::ContinueCallbackInfo const&)>;
using VarCallbackType = std::variant<JsOnCallbackType>;

class JsBaseObserver {
public:
    JsBaseObserver();
    virtual ~JsBaseObserver() {}
    bool SendEventToMainThread(const std::function<void()> func);

    std::recursive_mutex mutex_;
    static std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_;
};

class MissionObserver : public JsBaseObserver, public AAFwk::RemoteOnListenerStub,
    public std::enable_shared_from_this<MissionObserver> {
public:
    MissionObserver();
    ~MissionObserver();
    void AddCallback(VarCallbackType cb, ani_object callbackObj);
    void DeleteCallback(ani_ref jsCallbackRef);
    bool IsCallbackListEmpty();
    ani_ref CreateCallbackRefIfNotDuplicate(ani_object callbackObj);

    void OnCallback(const uint32_t continueState, const std::string &srcDeviceId,
        const std::string &bundleName, const std::string &continueType = "",
        const std::string &srcBundleName = "") override;
    void OnCallbackInMainThread(const ::ohos::distributedmissionmanager::ContinueCallbackInfo& info);

protected:
    std::vector<std::pair<VarCallbackType, ani_ref>> callbackList_;

};

}  // namespace ani_observerutils
#endif  // OHOS_ANI_OBSERVER_UTILS_H
