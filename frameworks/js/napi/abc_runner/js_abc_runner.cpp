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

#include "js_abc_runner.h"

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "js_runtime.h"
#include "native_engine/native_engine.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <mutex>
#include <unordered_map>

namespace OHOS {
namespace AbilityRuntime {

namespace {
    constexpr size_t ARGC_ZERO = 0;
    constexpr size_t ARGC_ONE = 1;
    constexpr size_t ARGC_TWO = 2;
    constexpr int32_t ERR_OK = 0;
    constexpr int32_t ERR_INVALID_PARAM = 401;
    constexpr int32_t ERR_FILE_NOT_EXIST = 16000001;
    constexpr int32_t ERR_INTERNAL = 16000050;
}

/**
 * Abc run result structure
 */
struct AbcRunResult {
    int32_t code = 0;
    std::string data;
    std::string message;
};

/**
 * Abc run callback function type
 */
using AbcRunCallback = std::function<void(const AbcRunResult&)>;

/**
 * Abc runner manager class
 * Manages abc execution and callbacks
 */
class AbcRunnerManager {
public:
    static AbcRunnerManager& GetInstance() {
        static AbcRunnerManager instance;
        return instance;
    }

    /**
     * Run abc file with specified parameters
     */
    int32_t RunAbc(napi_env env, const std::string& path, const std::string& paramsJson,
                   AbcRunCallback callback)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Check if file exists
        struct stat buffer;
        if (stat(path.c_str(), &buffer) != 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Abc file not found: %{public}s", path.c_str());
            return ERR_FILE_NOT_EXIST;
        }

        int32_t abcId = GenerateAbcId();
        currentAbcId_ = abcId;
        callbacks_[abcId] = callback;

        // Execute abc synchronously (not in background thread)
        ExecuteAbcInternal(env, abcId, path, paramsJson);

        return ERR_OK;
    }

    /**
     * Set result for current running abc
     */
    int32_t SetAbcResult(const AbcRunResult& result)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (currentAbcId_ == -1) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "No abc is currently running");
            return ERR_INTERNAL;
        }

        auto it = callbacks_.find(currentAbcId_);
        if (it != callbacks_.end() && it->second) {
            it->second(result);
            callbacks_.erase(it);
        }

        currentAbcId_ = -1;
        return ERR_OK;
    }

private:
    AbcRunnerManager() = default;
    ~AbcRunnerManager() = default;
    AbcRunnerManager(const AbcRunnerManager&) = delete;
    AbcRunnerManager& operator=(const AbcRunnerManager&) = delete;

    int32_t GenerateAbcId()
    {
        return nextAbcId_++;
    }

    void ExecuteAbcInternal(napi_env env, int32_t abcId, const std::string& path, const std::string& paramsJson)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Executing abc: %{public}s, id: %{public}d", path.c_str(), abcId);

        // Load and execute the abc file
        bool success = LoadAndExecuteAbc(env, abcId, path, paramsJson);
        
        if (!success) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (currentAbcId_ == abcId) {
                AbcRunResult result;
                result.code = ERR_INTERNAL;
                result.message = "Failed to execute abc";

                auto it = callbacks_.find(abcId);
                if (it != callbacks_.end() && it->second) {
                    it->second(result);
                    callbacks_.erase(it);
                }
                currentAbcId_ = -1;
            }
        }
    }

    bool LoadAndExecuteAbc(napi_env env, int32_t abcId, const std::string& path, const std::string& paramsJson)
    {
        // Get NativeEngine from env
        auto* engine = reinterpret_cast<NativeEngine*>(env);
        if (!engine) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "NativeEngine is null");
            return false;
        }

        TAG_LOGI(AAFwkTag::ABILITYMGR, "Loading and executing abc: %{public}s", path.c_str());

        // Open handle scope
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        if (scope == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to open handle scope");
            return false;
        }

        // Execute script using RunScriptPath (same as JsEnvironment::LoadScript)
        bool runSuccess = engine->RunScriptPath(path.c_str());
        
        if (!runSuccess) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to run script: %{public}s", path.c_str());
            napi_close_handle_scope(env, scope);
            return false;
        }

        TAG_LOGI(AAFwkTag::ABILITYMGR, "Script executed successfully: %{public}s", path.c_str());

        // Execute onStart callback
        bool result = ExecuteOnStart(env, abcId, paramsJson);
        
        napi_close_handle_scope(env, scope);
        return result;
    }

    bool ExecuteOnStart(napi_env env, int32_t abcId, const std::string& paramsJson)
    {
        HandleScope handleScope(env);

        // Get ScriptProcess from global object
        // The abc file should export: globalThis.ScriptProcess = { onStart: function(args) {...} }
        napi_value global = nullptr;
        napi_get_global(env, &global);
        
        napi_value scriptProcess = nullptr;
        napi_get_named_property(env, global, "ScriptProcess", &scriptProcess);

        if (scriptProcess == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "ScriptProcess not found in global, using default behavior");
            // If no ScriptProcess, just return success
            std::lock_guard<std::mutex> lock(mutex_);
            if (currentAbcId_ == abcId) {
                AbcRunResult result;
                result.code = 0;
                result.message = "Abc executed successfully (no ScriptProcess)";

                auto it = callbacks_.find(abcId);
                if (it != callbacks_.end() && it->second) {
                    it->second(result);
                    callbacks_.erase(it);
                }
                currentAbcId_ = -1;
            }
            return true;
        }

        // Get onStart method from ScriptProcess
        napi_value onStartFunc = nullptr;
        napi_get_named_property(env, scriptProcess, "onStart", &onStartFunc);

        if (onStartFunc == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "onStart method not found in ScriptProcess, using default behavior");
            // If no onStart, just return success
            std::lock_guard<std::mutex> lock(mutex_);
            if (currentAbcId_ == abcId) {
                AbcRunResult result;
                result.code = 0;
                result.message = "Abc executed successfully (no onStart)";

                auto it = callbacks_.find(abcId);
                if (it != callbacks_.end() && it->second) {
                    it->second(result);
                    callbacks_.erase(it);
                }
                currentAbcId_ = -1;
            }
            return true;
        }

        // Create args object from paramsJson
        napi_value args = nullptr;
        napi_create_object(env, &args);
        
        // TODO: Parse paramsJson and populate args object
        // For now, pass empty object

        // Store abcId for setAbcResult to use
        {
            std::lock_guard<std::mutex> lock(mutex_);
            currentAbcId_ = abcId;
        }

        // Call ScriptProcess.onStart(args)
        napi_value result = nullptr;
        napi_status status = napi_call_function(env, scriptProcess, onStartFunc, 1, &args, &result);

        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to call onStart, status: %{public}d", status);
            return false;
        }

        TAG_LOGI(AAFwkTag::ABILITYMGR, "onStart called successfully");
        return true;
    }

    std::mutex mutex_;
    int32_t currentAbcId_ = -1;
    int32_t nextAbcId_ = 1;
    std::unordered_map<int32_t, AbcRunCallback> callbacks_;
};

class JsAbcRunner {
public:
    JsAbcRunner() = default;
    ~JsAbcRunner() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        std::unique_ptr<JsAbcRunner>(static_cast<JsAbcRunner*>(data));
    }

    static napi_value RunAbc(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbcRunner, OnRunAbc);
    }

    static napi_value SetAbcResult(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbcRunner, OnSetAbcResult);
    }

private:
    napi_value OnRunAbc(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);

        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid parameter number");
            ThrowInvalidParamError(env, "Parameter error: The number of parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Parse options
        napi_value jsOptions = info.argv[0];
        std::string path;
        std::string paramsJson = "{}";

        if (!ParseRunOptions(env, jsOptions, path, paramsJson)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse options");
            ThrowInvalidParamError(env, "Parameter error: Invalid options.");
            return CreateJsUndefined(env);
        }

        auto complete = [env, path, paramsJson](napi_env callbackEnv, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(callbackEnv);
            
            auto callback = [&task, callbackEnv](const AbcRunResult& result) {
                HandleScope handleScope(callbackEnv);
                napi_value jsResult = CreateJsAbcRunResult(callbackEnv, result);
                if (result.code == 0) {
                    task.ResolveWithNoError(callbackEnv, jsResult);
                } else {
                    task.Reject(callbackEnv, CreateJsError(callbackEnv, result.code, result.message));
                }
            };

            int32_t ret = AbcRunnerManager::GetInstance().RunAbc(env, path, paramsJson, callback);
            if (ret != ERR_OK) {
                task.Reject(callbackEnv, CreateJsError(callbackEnv, ret, "Failed to run abc"));
            }
        };

        napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbcRunner::OnRunAbc", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnSetAbcResult(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);

        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid parameter number");
            ThrowInvalidParamError(env, "Parameter error: The number of parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Parse result
        AbcRunResult result;
        if (!ParseAbcRunResult(env, info.argv[0], result)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse result");
            ThrowInvalidParamError(env, "Parameter error: Invalid result.");
            return CreateJsUndefined(env);
        }

        int32_t ret = AbcRunnerManager::GetInstance().SetAbcResult(result);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to set abc result: %{public}d", ret);
            return CreateJsUndefined(env);
        }

        return handleEscape.Escape(CreateJsUndefined(env));
    }

    bool ParseRunOptions(napi_env env, napi_value jsOptions, std::string& path, 
                         std::string& paramsJson)
    {
        napi_value jsPath = nullptr;
        napi_get_named_property(env, jsOptions, "path", &jsPath);
        if (jsPath == nullptr || !ConvertFromJsValue(env, jsPath, path)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get path");
            return false;
        }

        // TODO: Parse params object to JSON string
        // For now, just use empty object
        paramsJson = "{}";

        return true;
    }

    bool ParseAbcRunResult(napi_env env, napi_value jsResult, AbcRunResult& result)
    {
        napi_value jsCode = nullptr;
        napi_get_named_property(env, jsResult, "code", &jsCode);
        if (jsCode != nullptr) {
            int32_t code = 0;
            if (ConvertFromJsValue(env, jsCode, code)) {
                result.code = code;
            }
        }

        napi_value jsMessage = nullptr;
        napi_get_named_property(env, jsResult, "message", &jsMessage);
        if (jsMessage != nullptr) {
            ConvertFromJsValue(env, jsMessage, result.message);
        }

        // TODO: Parse data object to JSON string
        result.data = "{}";

        return true;
    }

    napi_value CreateJsAbcRunResult(napi_env env, const AbcRunResult& result)
    {
        napi_value jsResult = nullptr;
        napi_create_object(env, &jsResult);

        napi_value jsCode = nullptr;
        napi_create_int32(env, result.code, &jsCode);
        napi_set_named_property(env, jsResult, "code", jsCode);

        if (!result.message.empty()) {
            napi_value jsMessage = nullptr;
            napi_create_string_utf8(env, result.message.c_str(), result.message.length(), &jsMessage);
            napi_set_named_property(env, jsResult, "message", jsMessage);
        }

        // TODO: Parse data JSON string to JS object
        napi_value jsData = nullptr;
        napi_create_object(env, &jsData);
        napi_set_named_property(env, jsResult, "data", jsData);

        return jsResult;
    }
};

napi_value CreateJsAbcRunner(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    
    napi_value runAbc = nullptr;
    napi_create_function(env, "runAbc", NAPI_AUTO_LENGTH, JsAbcRunner::RunAbc,
                         nullptr, &runAbc);
    napi_set_named_property(env, exportObj, "runAbc", runAbc);

    napi_value setAbcResult = nullptr;
    napi_create_function(env, "setAbcResult", NAPI_AUTO_LENGTH, JsAbcRunner::SetAbcResult,
                         nullptr, &setAbcResult);
    napi_set_named_property(env, exportObj, "setAbcResult", setAbcResult);

    return exportObj;
}

} // namespace AbilityRuntime
} // namespace OHOS
