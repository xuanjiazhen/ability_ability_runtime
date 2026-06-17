/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H

#include <cstdint>
#include <memory>
#include <sstream>
#include <type_traits>

#include "napi/native_api.h"
#include "native_engine/native_engine.h"

#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_MAX_COUNT = 10;

#define NAPI_CALL_NO_THROW(theCall, retVal)      \
    do {                                         \
        if ((theCall) != napi_ok) {              \
            return retVal;                       \
        }                                        \
    } while (0)

/*
 * GET_CB_INFO_AND_CALL 是一个宏，用来简化 N-API 静态入口函数的写法。
 *
 * 参数说明：
 *   env  ：N-API 环境。
 *   info ：JS 调用 native 函数时传进来的调用信息，里面包含参数、this 对象等。
 *   T    ：当前 native 类类型，例如 JSCliManager。
 *   func ：真正要调用的成员函数名，例如 OnExecTool。
 *
 * 作用：
 *   1. 从 napi_callback_info 中解析 JS 参数；
 *   2. 从 JS this 对象中取出之前 napi_wrap 绑定的 C++ 对象；
 *   3. 调用该对象的成员函数 func(env, argc, argv)。
 */
#define GET_CB_INFO_AND_CALL(env, info, T, func)                                       \
    do {                                                                               \
        /*                                                                              \
         * argc 表示 JS 传入参数的数量。                                                  \
         * 初始值设为 ARGC_MAX_COUNT，表示最多接收这么多个参数。                         \
         * GetCbInfoFromCallbackInfo 执行后，argc 会被更新成实际参数个数。               \
         */                                                                             \
        size_t argc = ARGC_MAX_COUNT;                                                  \
                                                                                       \
        /*                                                                              \
         * argv 用来保存 JS 侧传入的参数。                                                 \
         * 每个参数都是 napi_value 类型。                                                  \
         * 例如 JS 调用 execTool(a, b, c)，那么 argv[0]=a, argv[1]=b, argv[2]=c。          \
         */                                                                             \
        napi_value argv[ARGC_MAX_COUNT] = {nullptr};                                   \
                                                                                       \
        /*                                                                              \
         * GetCbInfoFromCallbackInfo 会做两件核心事情：                                  \
         *   1. 从 info 中解析出 argc / argv；                                            \
         *   2. 从 JS this 对象中 unwrap 出之前 napi_wrap 绑定的 C++ 对象指针。           \
         *                                                                                \
         * 返回值是 void*，所以这里用 static_cast<T*> 转成具体类型 T*。                  \
         * 例如 T 是 JSCliManager，那么 me 就是 JSCliManager*。                         \
         */                                                                             \
        T* me = static_cast<T*>(GetCbInfoFromCallbackInfo(env, info, &argc, argv));    \
                                                                                       \
        /*                                                                              \
         * 如果成功取到 C++ 对象 me，就调用真正的成员函数：                               \
         *   me->func(env, argc, argv)                                                    \
         *                                                                                \
         * 例如：                                                                         \
         *   me->OnExecTool(env, argc, argv)                                              \
         *                                                                                \
         * 如果 me 为空，说明没有成功取到 C++ 对象，直接返回 nullptr。                    \
         */                                                                             \
        return (me != nullptr) ? me->func(env, argc, argv) : nullptr;                  \
    } while (0)
    
struct NapiCallbackInfo {
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
};

#define GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, T, func, name)                         \
    do {                                                                                   \
        NapiCallbackInfo napiInfo;                                                         \
        T* me = static_cast<T*>(GetNapiCallbackInfoAndThis(env, info, napiInfo, name));    \
        return (me != nullptr) ? me->func(env, napiInfo) : nullptr;                        \
    } while (0)

#define GET_NAPI_INFO_AND_CALL(env, info, T, func)                                         \
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, T, func, nullptr)

void* GetNapiCallbackInfoAndThis(
    napi_env env, napi_callback_info info, NapiCallbackInfo& napiInfo, const char* name = nullptr);

template<typename T, size_t N>
inline constexpr size_t ArraySize(T (&)[N]) noexcept
{
    return N;
}

inline napi_value CreateJsUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

inline napi_value CreateJsNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

inline napi_value CreateJsNumber(napi_env env, int32_t value)
{
    napi_value result = nullptr;
    napi_create_int32(env, value, &result);
    return result;
}

inline napi_value CreateJsNumber(napi_env env, uint32_t value)
{
    napi_value result = nullptr;
    napi_create_uint32(env, value, &result);
    return result;
}

inline napi_value CreateJsNumber(napi_env env, int64_t value)
{
    napi_value result = nullptr;
    napi_create_int64(env, value, &result);
    return result;
}

inline napi_value CreateJsNumber(napi_env env, double value)
{
    napi_value result = nullptr;
    napi_create_double(env, value, &result);
    return result;
}

template<class T>
inline napi_value CreateJsValue(napi_env env, const T& value)
{
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    napi_value result = nullptr;
    if constexpr (std::is_same_v<ValueType, bool>) {
        napi_get_boolean(env, value, &result);
        return result;
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        return CreateJsNumber(env, value);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        napi_create_string_utf8(env, value.c_str(), value.length(), &result);
        return result;
    } else if constexpr (std::is_enum_v<ValueType>) {
        return CreateJsNumber(env, static_cast<std::make_signed_t<ValueType>>(value));
    } else if constexpr (std::is_same_v<ValueType, const char*>) {
        (value != nullptr) ? napi_create_string_utf8(env, value, strlen(value), &result) :
            napi_get_undefined(env, &result);
        return result;
    }
}

inline bool ConvertFromJsNumber(napi_env env, napi_value jsValue, int32_t& value)
{
    NAPI_CALL_NO_THROW(napi_get_value_int32(env, jsValue, &value), false);
    return true;
}

inline bool ConvertFromJsNumber(napi_env env, napi_value jsValue, uint32_t& value)
{
    NAPI_CALL_NO_THROW(napi_get_value_uint32(env, jsValue, &value), false);
    return true;
}

inline bool ConvertFromJsNumber(napi_env env, napi_value jsValue, int64_t& value)
{
    NAPI_CALL_NO_THROW(napi_get_value_int64(env, jsValue, &value), false);
    return true;
}

inline bool ConvertFromJsNumber(napi_env env, napi_value jsValue, double& value)
{
    NAPI_CALL_NO_THROW(napi_get_value_double(env, jsValue, &value), false);
    return true;
}

template<class T>
inline bool ConvertFromJsValue(napi_env env, napi_value jsValue, T& value)
{
    if (jsValue == nullptr) {
        return false;
    }

    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<ValueType, bool>) {
        NAPI_CALL_NO_THROW(napi_get_value_bool(env, jsValue, &value), false);
        return true;
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        return ConvertFromJsNumber(env, jsValue, value);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        size_t len = 0;
        NAPI_CALL_NO_THROW(napi_get_value_string_utf8(env, jsValue, nullptr, 0, &len), false);
        auto buffer = std::make_unique<char[]>(len + 1);
        size_t strLength = 0;
        NAPI_CALL_NO_THROW(napi_get_value_string_utf8(env, jsValue, buffer.get(), len + 1, &strLength), false);
        value = buffer.get();
        return true;
    } else if constexpr (std::is_enum_v<ValueType>) {
        std::make_signed_t<ValueType> numberValue = 0;
        if (!ConvertFromJsNumber(env, jsValue, numberValue)) {
            return false;
        }
        value = static_cast<ValueType>(numberValue);
        return true;
    }
}

template<class T>
napi_value CreateNativeArray(napi_env env, const std::vector<T>& data)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, data.size(), &arrayValue);
    uint32_t index = 0;
    for (const T& item : data) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, item));
    }
    return arrayValue;
}

napi_value CreateJsError(napi_env env, int32_t errCode, const std::string& message = std::string());
void BindNativeFunction(napi_env env, napi_value object, const char* name,
    const char* moduleName, napi_callback func);
void BindNativeProperty(napi_env env, napi_value object, const char* name, napi_callback getter);
void* GetNativePointerFromCallbackInfo(napi_env env, napi_callback_info info, const char* name);
void* GetCbInfoFromCallbackInfo(napi_env env, napi_callback_info info, size_t* argc, napi_value* argv);

void SetNamedNativePointer(
    napi_env env, napi_value object, const char* name, void* ptr, napi_finalize func);
void* GetNamedNativePointer(napi_env env, napi_value object, const char* name);

bool CheckTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType);

void ReleaseNativeReference(napi_env env, NativeReference* ref);

template<class T>
T* CheckParamsAndGetThis(napi_env env, napi_callback_info info, const char* name = nullptr)
{
    return static_cast<T*>(GetNativePointerFromCallbackInfo(env, info, name));
}

class HandleScope final {
public:
    explicit HandleScope(JsRuntime& jsRuntime);
    explicit HandleScope(napi_env env);
    ~HandleScope();

    HandleScope(const HandleScope&) = delete;
    HandleScope(HandleScope&&) = delete;
    HandleScope& operator=(const HandleScope&) = delete;
    HandleScope& operator=(HandleScope&&) = delete;
private:
    napi_handle_scope scope_ = nullptr;
    napi_env env_ = nullptr;
};

class HandleEscape final {
public:
    explicit HandleEscape(JsRuntime& jsRuntime);
    explicit HandleEscape(napi_env env);
    ~HandleEscape();

    napi_value Escape(napi_value value);

    HandleEscape(const HandleEscape&) = delete;
    HandleEscape(HandleEscape&&) = delete;
    HandleEscape& operator=(const HandleEscape&) = delete;
    HandleEscape& operator=(HandleEscape&&) = delete;
private:
    napi_escapable_handle_scope scope_ = nullptr;
    napi_env env_ = nullptr;
};

class NapiAsyncTask final {
public:
    using ExecuteCallback = std::function<void()>;
    using CompleteCallback = std::function<void(napi_env, NapiAsyncTask&, int32_t)>;

    static void Schedule(const std::string& name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task);
    static void ScheduleWithDefaultQos(const std::string &name, napi_env env,
        std::unique_ptr<NapiAsyncTask>&& task);
    static void ScheduleHighQos(const std::string& name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task);
    static void ScheduleLowQos(const std::string& name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task);
    bool StartWithDefaultQos(const std::string &name, napi_env env);

    NapiAsyncTask(napi_deferred deferred, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    NapiAsyncTask(napi_ref callbackRef, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    ~NapiAsyncTask();

    void Resolve(napi_env env, napi_value value);
    void Reject(napi_env env, napi_value error);
    void ResolveWithNoError(napi_env env, napi_value value);
    void ResolveWithCustomize(napi_env env, napi_value error, napi_value value);
    void RejectWithCustomize(napi_env env, napi_value error, napi_value value);
private:
    static void Execute(napi_env env, void* data);
    static void Complete(napi_env env, napi_status status, void* data);

    bool Start(const std::string &name, napi_env env);
    bool StartHighQos(const std::string &name, napi_env env);
    bool StartLowQos(const std::string &name, napi_env env);
    bool SendNapiEvent(napi_env env, napi_event_priority eventPriority);

    napi_deferred deferred_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    napi_async_work work_ = nullptr;
    std::unique_ptr<ExecuteCallback> execute_;
    std::unique_ptr<CompleteCallback> complete_;
    napi_env env_ = nullptr;
};

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback&& execute, NapiAsyncTask::CompleteCallback&& complete, napi_value* result);

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback&& execute, std::nullptr_t, napi_value* result);

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    std::nullptr_t, NapiAsyncTask::CompleteCallback&& complete, napi_value* result);

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    std::nullptr_t, std::nullptr_t, napi_value* result);

std::unique_ptr<NapiAsyncTask> CreateEmptyAsyncTask(napi_env env, napi_value lastParam, napi_value* result);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H
