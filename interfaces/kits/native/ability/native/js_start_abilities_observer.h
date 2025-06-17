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

 #ifndef OHOS_ABILITY_RUNTIME_JS_START_ABILITIES_OBSERVER_H
 #define OHOS_ABILITY_RUNTIME_JS_START_ABILITIES_OBSERVER_H
 
 #include <memory>
 #include <string>
 #include <vector>
 
 #include "native_engine/native_engine.h"
 #include "js_runtime_utils.h"
 
 namespace OHOS {
 namespace AbilityRuntime {
 struct JsStartAbilitiesObserverObject {
     std::string startTime;
     int32_t resultCode;
     napi_deferred deferred;
 };
 
 class JsStartAbilitiesObserver {
 public:
     /**
      * JsStartAbilitiesObserver, constructor.
      *
      */
     explicit JsStartAbilitiesObserver(napi_env env);
 
     /**
      * JsStartAbilitiesObserver, destructor.
      *
      */
     ~JsStartAbilitiesObserver();
 
     /**
      * OnInstallFinished, return free install result.
      *
      * @param bundleName Free install bundleName.
      * @param abilityName Free install abilityName.
      * @param startTime Free install start request time.
      * @param resultCode The result of this free install.
      */
     void OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
         const std::string &startTime, const int &resultCode) override;
 
     /**
      * @brief Use for context to add an callback into the observer.
      *
      * @param bundleName The bundleName of want.
      * @param abilityName The abilityName of want.
      * @param startTime The startTime that want requested.
      * @param jsObserverObject The js object instance.
      * @param result the promise to return.
      */
     void AddJsObserverObject(const std::string &bundleName, const std::string &abilityName,
         const std::string &startTime, napi_value jsObserverObject, napi_value* result, bool isAbilityResult = false);
 
 private:
     /**
      * CallPromise, resolve promise.
      *
      * @param deferred The promise that is to be resolved.
      * @param resultCode The result code of the promise.
      */
     void CallPromise(napi_deferred deferred, int32_t resultCode);
 
     /**
      * HandleOnInstallFinished, handle the event of free install upon finished.
      *
      * @param bundleName The bundle name of the app.
      * @param abilityName The ability name of the app.
      * @param startTime The start time.
      * @param resultCode The result code.
      */
     void HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
         const std::string &startTime, const int &resultCode);
 
     /**
      * AddJsObserverCommon, helper of AddJsObserverObject.
      *
      * @param object The free install observer object.
      * @param jsObserverObject The js observer object.
      * @param result The resultant object.
      * @param isAbilityResult The flag of whether it is to return ability result.
      */
     void AddJsObserverCommon(JsStartAbilitiesObserverObject &object,
         napi_value jsObserverObject, napi_value* result, bool isAbilityResult);
     napi_env env_;
     std::mutex jsObserverObjectListLock_;
     std::vector<JsStartAbilitiesObserverObject> jsObserverObjectList_;
 };
 } // namespace AbilityRuntime
 } // namespace OHOS
 
 #endif // OHOS_ABILITY_RUNTIME_JS_START_ABILITIES_OBSERVER_H