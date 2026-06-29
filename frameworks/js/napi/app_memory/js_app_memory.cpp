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

#include "js_app_memory.h"

#include <memory>
#include <string>
#include <vector>

#include "ability_runtime_error_util.h"
#include "file_page_releaser.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "napi/native_api.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
using Security::AccessToken::TokenIdKit;
using FilePageReleaser::ERR_OK;
using FilePageReleaser::ERR_NOT_SYSTEM_APP;
using FilePageReleaser::ERR_INVALID_VALUE;
using FilePageReleaser::ERR_INTERNAL_ERROR;

constexpr size_t ARGC_ONE = 1;

bool CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITY, "appMemory: not system app");
        return false;
    }
    return true;
}

// Throw the ability_runtime error code and return undefined (synchronous fail,
// no promise). Matches the convention used by other system-only APIs.
napi_value ThrowAndReturnUndefined(napi_env env, int32_t errCode, const std::string& msg)
{
    AbilityRuntimeErrorUtil::Throw(env, errCode, msg);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    return undefined;
}

struct ReleaseFilePagesContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    std::vector<std::string> fileNames;
    int32_t errCode = ERR_OK;
};

struct ReleaseRawFileContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    std::string moduleName;
    int32_t errCode = ERR_OK;
};

void DestroyContext(napi_env env, ReleaseFilePagesContext* ctx)
{
    if (ctx != nullptr && ctx->work != nullptr) {
        napi_delete_async_work(env, ctx->work);
        ctx->work = nullptr;
    }
    delete ctx;
}

void DestroyRawContext(napi_env env, ReleaseRawFileContext* ctx)
{
    if (ctx != nullptr && ctx->work != nullptr) {
        napi_delete_async_work(env, ctx->work);
        ctx->work = nullptr;
    }
    delete ctx;
}
} // namespace

napi_value ReleaseFilePages(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (!CheckCallerIsSystemApp()) {
        return ThrowAndReturnUndefined(env, ERR_NOT_SYSTEM_APP, "appMemory is a system API.");
    }
    if (argc < ARGC_ONE) {
        return ThrowAndReturnUndefined(env, ERR_INVALID_VALUE, "Parameter error. fileNames required.");
    }

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[0], &isArray));
    if (!isArray) {
        return ThrowAndReturnUndefined(env, ERR_INVALID_VALUE, "Parameter error. fileNames must be an array.");
    }
    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[0], &length));
    if (length == 0) {
        return ThrowAndReturnUndefined(env, ERR_INVALID_VALUE, "Parameter error. fileNames is empty.");
    }

    auto* ctx = new (std::nothrow) ReleaseFilePagesContext();
    if (ctx == nullptr) {
        return ThrowAndReturnUndefined(env, ERR_INTERNAL_ERROR, "Internal error.");
    }
    bool bad = false;
    for (uint32_t i = 0; i < length; i++) {
        napi_value elem = nullptr;
        napi_get_element(env, argv[0], i, &elem);
        if (elem == nullptr) {
            bad = true;
            break;
        }
        napi_valuetype vt = napi_undefined;
        napi_typeof(env, elem, &vt);
        if (vt != napi_string) {
            bad = true;
            break;
        }
        size_t len = 0;
        napi_get_value_string_utf8(env, elem, nullptr, 0, &len);
        std::string s;
        s.resize(len);
        napi_get_value_string_utf8(env, elem, s.data(), len + 1, &len);
        if (s.empty()) {
            bad = true;
            break;
        }
        ctx->fileNames.push_back(std::move(s));
    }
    if (bad) {
        delete ctx;
        return ThrowAndReturnUndefined(env, ERR_INVALID_VALUE, "Parameter error. each fileName must be a string.");
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "ReleaseFilePages", NAPI_AUTO_LENGTH, &resourceName);
    napi_value promise = nullptr;
    napi_create_promise(env, &ctx->deferred, &promise);

    napi_execute_callback execute = [](napi_env env, void* data) {
        auto* c = static_cast<ReleaseFilePagesContext*>(data);
        c->errCode = FilePageReleaser::ReleaseByFileNames(c->fileNames);
    };
    napi_complete_callback complete = [](napi_env env, napi_status status, void* data) {
        auto* c = static_cast<ReleaseFilePagesContext*>(data);
        if (c->errCode == ERR_OK) {
            napi_value undefined = nullptr;
            napi_get_undefined(env, &undefined);
            napi_resolve_deferred(env, c->deferred, undefined);
        } else {
            std::string msg = "release failed, code=" + std::to_string(c->errCode);
            napi_value errCode = nullptr;
            napi_create_int32(env, c->errCode, &errCode);
            napi_value errMsg = nullptr;
            napi_create_string_utf8(env, msg.c_str(), NAPI_AUTO_LENGTH, &errMsg);
            napi_value error = nullptr;
            napi_create_error(env, nullptr, errMsg, &error);
            napi_set_named_property(env, error, "code", errCode);
            napi_reject_deferred(env, c->deferred, error);
        }
        DestroyContext(env, c);
    };
    napi_create_async_work(env, nullptr, resourceName, execute, complete, ctx, &ctx->work);
    napi_queue_async_work(env, ctx->work);
    return promise;
}

napi_value ReleaseRawFilePages(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (!CheckCallerIsSystemApp()) {
        return ThrowAndReturnUndefined(env, ERR_NOT_SYSTEM_APP, "appMemory is a system API.");
    }
    if (argc < ARGC_ONE) {
        return ThrowAndReturnUndefined(env, ERR_INVALID_VALUE, "Parameter error. moduleName required.");
    }
    napi_valuetype vt = napi_undefined;
    napi_typeof(env, argv[0], &vt);
    if (vt != napi_string) {
        return ThrowAndReturnUndefined(env, ERR_INVALID_VALUE, "Parameter error. moduleName must be a string.");
    }
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (len == 0) {
        return ThrowAndReturnUndefined(env, ERR_INVALID_VALUE, "Parameter error. moduleName is empty.");
    }

    auto* ctx = new (std::nothrow) ReleaseRawFileContext();
    if (ctx == nullptr) {
        return ThrowAndReturnUndefined(env, ERR_INTERNAL_ERROR, "Internal error.");
    }
    ctx->moduleName.resize(len);
    napi_get_value_string_utf8(env, argv[0], ctx->moduleName.data(), len + 1, &len);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "ReleaseRawFilePages", NAPI_AUTO_LENGTH, &resourceName);
    napi_value promise = nullptr;
    napi_create_promise(env, &ctx->deferred, &promise);

    napi_execute_callback execute = [](napi_env env, void* data) {
        auto* c = static_cast<ReleaseRawFileContext*>(data);
        c->errCode = FilePageReleaser::ReleaseRawFileByModule(c->moduleName);
    };
    napi_complete_callback complete = [](napi_env env, napi_status status, void* data) {
        auto* c = static_cast<ReleaseRawFileContext*>(data);
        if (c->errCode == ERR_OK) {
            napi_value undefined = nullptr;
            napi_get_undefined(env, &undefined);
            napi_resolve_deferred(env, c->deferred, undefined);
        } else {
            std::string msg = "release rawfile failed, code=" + std::to_string(c->errCode);
            napi_value errCode = nullptr;
            napi_create_int32(env, c->errCode, &errCode);
            napi_value errMsg = nullptr;
            napi_create_string_utf8(env, msg.c_str(), NAPI_AUTO_LENGTH, &errMsg);
            napi_value error = nullptr;
            napi_create_error(env, nullptr, errMsg, &error);
            napi_set_named_property(env, error, "code", errCode);
            napi_reject_deferred(env, c->deferred, error);
        }
        DestroyRawContext(env, c);
    };
    napi_create_async_work(env, nullptr, resourceName, execute, complete, ctx, &ctx->work);
    napi_queue_async_work(env, ctx->work);
    return promise;
}

napi_value JsAppMemoryInit(napi_env env, napi_value exportObj)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("releaseFilePages", ReleaseFilePages),
        DECLARE_NAPI_FUNCTION("releaseRawFilePages", ReleaseRawFilePages),
    };
    NAPI_CALL(env, napi_define_properties(env, exportObj, sizeof(desc) / sizeof(desc[0]), desc));
    return exportObj;
}
} // namespace AbilityRuntime
} // namespace OHOS
