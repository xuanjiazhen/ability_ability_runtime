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

#include "js_skill_func.h"

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "native_reference.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

std::shared_ptr<JsSkillFunc> JsSkillFunc::Create(JsRuntime &runtime)
{
    return std::make_shared<JsSkillFunc>(runtime);
}

JsSkillFunc::JsSkillFunc(JsRuntime &runtime) : runtime_(runtime) {}

JsSkillFunc::~JsSkillFunc()
{
    if (moduleRef_ != nullptr) {
        auto *env = runtime_.GetNapiEnv();
        if (env != nullptr) {
            napi_delete_reference(env, moduleRef_);
        }
        moduleRef_ = nullptr;
    }
}

bool JsSkillFunc::Init(const SkillExecutorInfo &skillInfo)
{
    funcName_ = skillInfo.funcName.empty() ? "ExecuteSkill" : skillInfo.funcName;
    return LoadModule(skillInfo);
}

bool JsSkillFunc::LoadModule(const SkillExecutorInfo &skillInfo)
{
    auto executeParam = skillInfo.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null executeParam");
        return false;
    }

    auto *env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return false;
    }

    std::string moduleName = executeParam->moduleName_;
    std::string srcEntrance = executeParam->srcEntrance_;

    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "load skill module, moduleName:%{public}s srcEntrance:%{private}s",
        moduleName.c_str(), srcEntrance.c_str());

    auto safeData = runtime_.ExecuteSecureWithOhmUrl(moduleName, "", srcEntrance);
    if (safeData == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "load module failed");
        return false;
    }

    napi_value moduleObj = runtime_.GetExportObjectFromOhmUrl(srcEntrance, "");
    if (moduleObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get export object failed");
        return false;
    }

    napi_create_reference(env, moduleObj, 1, &moduleRef_);
    return moduleRef_ != nullptr;
}

bool JsSkillFunc::ExecuteSkill(
    std::shared_ptr<SkillExecuteParam> skillParam,
    std::unique_ptr<SkillExecutorAsyncCallback> callback,
    bool &isAsync)
{
    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null callback");
        return false;
    }
    callback_ = std::move(callback);
    return CallExecuteSkill(skillParam, isAsync);
}

bool JsSkillFunc::CallExecuteSkill(const std::shared_ptr<SkillExecuteParam> &skillParam, bool &isAsync)
{
    auto *env = runtime_.GetNapiEnv();
    if (env == nullptr || moduleRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env or module");
        ReplyFailed();
        return false;
    }

    napi_value moduleObj = nullptr;
    napi_get_reference_value(env, moduleRef_, &moduleObj);
    if (moduleObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null module object");
        ReplyFailed();
        return false;
    }

    napi_value func = nullptr;
    napi_get_named_property(env, moduleObj, funcName_.c_str(), &func);
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s function not found", funcName_.c_str());
        ReplyFailed();
        return false;
    }

    napi_valuetype funcType = napi_undefined;
    napi_typeof(env, func, &funcType);
    if (funcType != napi_function) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s is not a function", funcName_.c_str());
        ReplyFailed();
        return false;
    }

    napi_value argv[2] = {nullptr};
    size_t argc = 0;
    if (skillParam != nullptr && skillParam->params_ != nullptr) {
        argv[0] = AppExecFwk::WrapWantParams(env, *skillParam->params_);
        argc = 1;
    }

    napi_value result = nullptr;
    napi_status status = napi_call_function(env, moduleObj, func, argc, argv, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "call %{public}s failed: %{public}d", funcName_.c_str(), status);
        ReplyFailed();
        return false;
    }

    return HandleResult(result, isAsync);
}

bool JsSkillFunc::HandleResult(napi_value result, bool &isAsync)
{
    auto *env = runtime_.GetNapiEnv();
    if (result == nullptr || env == nullptr) {
        ReplyFailed();
        return false;
    }

    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    isAsync = isPromise;

    if (isPromise) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteSkill returned Promise");
        auto *cb = callback_.release();

        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        napi_value resolveCb = nullptr;
        napi_create_function(env, "_", strlen("_"), OnPromiseResolve, cb, &resolveCb);
        napi_value argvThen[1] = { resolveCb };
        napi_call_function(env, result, then, 1, argvThen, nullptr);

        napi_value catchFn = nullptr;
        napi_get_named_property(env, result, "catch", &catchFn);
        napi_value rejectCb = nullptr;
        napi_create_function(env, "_", strlen("_"), OnPromiseReject, cb, &rejectCb);
        napi_value argvCatch[1] = { rejectCb };
        napi_call_function(env, result, catchFn, 1, argvCatch, nullptr);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteSkill returned sync result");
        napi_valuetype type = napi_undefined;
        napi_typeof(env, result, &type);
        std::string resultStr;
        if (type == napi_string) {
            size_t len = 0;
            napi_get_value_string_utf8(env, result, nullptr, 0, &len);
            resultStr.resize(len);
            napi_get_value_string_utf8(env, result, resultStr.data(), len + 1, &len);
        } else if (type == napi_object) {
            napi_value json = nullptr;
            napi_get_named_property(env, result, "JSON", &json);
            if (json != nullptr) {
                napi_value stringify = nullptr;
                napi_get_named_property(env, json, "stringify", &stringify);
                if (stringify != nullptr) {
                    napi_value args[1] = { result };
                    napi_value jsonStr = nullptr;
                    napi_call_function(env, json, stringify, 1, args, &jsonStr);
                    if (jsonStr != nullptr) {
                        size_t len = 0;
                        napi_get_value_string_utf8(env, jsonStr, nullptr, 0, &len);
                        resultStr.resize(len);
                        napi_get_value_string_utf8(env, jsonStr, resultStr.data(), len + 1, &len);
                    }
                }
            }
        }
        ReplySucceeded(resultStr);
    }
    return true;
}

void JsSkillFunc::ReplyFailed()
{
    auto *cb = callback_.release();
    if (cb != nullptr) {
        SkillExecuteResult result;
        result.code = ERR_INVALID_VALUE;
        cb->Call(result);
        delete cb;
    }
}

void JsSkillFunc::ReplySucceeded(const std::string &resultStr)
{
    auto *cb = callback_.release();
    if (cb != nullptr) {
        SkillExecuteResult result;
        result.code = ERR_OK;
        result.result = std::make_shared<AppExecFwk::WantParams>();
        if (!resultStr.empty()) {
            result.result->SetParam("methodResult", AAFwk::String::Box(resultStr));
        }
        cb->Call(result);
        delete cb;
    }
}

void JsSkillFunc::OnPromiseResolve(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);

    auto *cb = static_cast<SkillExecutorAsyncCallback *>(data);
    if (cb == nullptr) {
        return;
    }

    std::string resultStr;
    if (argv[0] != nullptr) {
        napi_valuetype type = napi_undefined;
        napi_typeof(env, argv[0], &type);
        if (type == napi_string) {
            size_t len = 0;
            napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
            resultStr.resize(len);
            napi_get_value_string_utf8(env, argv[0], resultStr.data(), len + 1, &len);
        } else if (type == napi_object) {
            napi_value json = nullptr;
            napi_get_named_property(env, argv[0], "JSON", &json);
            if (json != nullptr) {
                napi_value stringify = nullptr;
                napi_get_named_property(env, json, "stringify", &stringify);
                if (stringify != nullptr) {
                    napi_value args[1] = { argv[0] };
                    napi_value jsonStr = nullptr;
                    napi_call_function(env, json, stringify, 1, args, &jsonStr);
                    if (jsonStr != nullptr) {
                        size_t len = 0;
                        napi_get_value_string_utf8(env, jsonStr, nullptr, 0, &len);
                        resultStr.resize(len);
                        napi_get_value_string_utf8(env, jsonStr, resultStr.data(), len + 1, &len);
                    }
                }
            }
        }
    }

    SkillExecuteResult result;
    result.code = ERR_OK;
    result.result = std::make_shared<AppExecFwk::WantParams>();
    if (!resultStr.empty()) {
        result.result->SetParam("methodResult", AAFwk::String::Box(resultStr));
    }
    cb->Call(result);
    delete cb;
}

void JsSkillFunc::OnPromiseReject(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);

    auto *cb = static_cast<SkillExecutorAsyncCallback *>(data);
    if (cb == nullptr) {
        return;
    }

    SkillExecuteResult result;
    result.code = ERR_INVALID_VALUE;
    cb->Call(result);
    delete cb;
}

} // namespace AbilityRuntime
} // namespace OHOS
