/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_INFOS_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_INFOS_H

#include <string>
#include "ability_lifecycle_executor.h"
#include "iremote_object.h"
#include "sts_runtime.h"

class NativeReference;

namespace OHOS {
namespace AppExecFwk {

struct BaseDelegatorAbilityProperty {
    // token of ability
    sptr<IRemoteObject> token_;
    // name of ability
    std::string name_;
    // name of modulename
    std::string moduleName_;
    // name of bundle + ability
    std::string fullName_;
    // lifecycle state of ability
    AbilityLifecycleExecutor::LifecycleState lifecycleState_ {AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED};

    std::weak_ptr<void> object_;

    virtual void SetRuntimeObject(const std::shared_ptr<void>& obj) = 0;

    virtual std::shared_ptr<void> GetRuntimeObject() const = 0;

    virtual ~BaseDelegatorAbilityProperty() = default;

};
struct ADelegatorAbilityProperty : public BaseDelegatorAbilityProperty {

    std::weak_ptr<NativeReference> object_;

    void SetRuntimeObject(const std::shared_ptr<void>& obj) override {
        auto nativeRef = std::static_pointer_cast<NativeReference>(obj);
        object_ = nativeRef;
    }
    std::shared_ptr<void> GetRuntimeObject() const override {
        return object_.lock();
    }
};

struct ETSDelegatorAbilityProperty  : public BaseDelegatorAbilityProperty {

    std::weak_ptr<AbilityRuntime::STSNativeReference> object_;

    void SetRuntimeObject(const std::shared_ptr<void>& obj) override {
        auto stsRef = std::static_pointer_cast<AbilityRuntime::STSNativeReference>(obj);
        object_ = stsRef;
    }
    std::shared_ptr<void> GetRuntimeObject() const override {
        return object_.lock();
    }
};

struct BaseDelegatorAbilityStageProperty {
    std::string moduleName_;
    std::string srcEntrance_;
    std::weak_ptr<void> object_;
    virtual void SetRuntimeObject(const std::shared_ptr<void>& obj) = 0;
    virtual std::shared_ptr<void> GetRuntimeObject() const = 0;
    virtual ~BaseDelegatorAbilityStageProperty() = default;
};

struct DelegatorAbilityStageProperty  : public BaseDelegatorAbilityStageProperty {
    std::weak_ptr<NativeReference> object_;
    void SetRuntimeObject(const std::shared_ptr<void>& obj) override {
        auto nativeRef = std::static_pointer_cast<NativeReference>(obj);
        object_ = nativeRef;
    }
    std::shared_ptr<void> GetRuntimeObject() const override {
        return object_.lock();
    }
};

struct ETSDelegatorAbilityStageProperty  : public BaseDelegatorAbilityStageProperty {
    std::weak_ptr<AbilityRuntime::STSNativeReference> object_;
    void SetRuntimeObject(const std::shared_ptr<void>& obj) override {
        auto stsRef = std::static_pointer_cast<AbilityRuntime::STSNativeReference>(obj);
        object_ = stsRef;
    }
    std::shared_ptr<void> GetRuntimeObject() const override {
        return object_.lock();
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_INFOS_H
