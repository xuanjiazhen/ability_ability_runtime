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
#include "napi/native_api.h"
#include "native_common.h"
#include <ark_runtime/jsvm.h>
#include "napi/NapiJsvmErrorCodeTest.h"

EXTERN_C_START

typedef struct {
    const char* name;
    napi_callback cb;
} FunctionDef;

static FunctionDef jsvm_functions1[] = {
    {"testCreateVMJSOK", testCreateVMJSOK},
    {"testOpenVMScopeJSOK", testOpenVMScopeJSOK},
    {"testCloseVMScopeJSOK", testCloseVMScopeJSOK},
    {"testCreateProxyJSOK", testCreateProxyJSOK},
    {"testCreateProxyInvalid", testCreateProxyInvalid},
    {"testCreateProxyObejct", testCreateProxyObejct},
    {"testCreateProxyPending", testCreateProxyPending},
    {"testCreateEnvJSOK", testCreateEnvJSOK},
    {"testCreateEnvFromSnapshotJSOK", testCreateEnvFromSnapshotJSOK},
    {"testDestroyEnvJSOK", testDestroyEnvJSOK},
    {"testOpenEnvScopeJSOK", testOpenEnvScopeJSOK},
    {"testCloseEnvScopeJSOK", testCloseEnvScopeJSOK},
    {"testGetVMJSOK", testGetVMJSOK},
    {"testIsProxyJSOK", testIsProxyJSOK},
    {"testIsProxyInvalid", testIsProxyInvalid},
    {"testProxyGetTargetJSOK", testProxyGetTargetJSOK},
    {"testProxyGetTargetInvalid", testProxyGetTargetInvalid},
    {"testProxyGetTargetInvalidType", testProxyGetTargetInvalidType},
    {"testCompileScriptJSOK", testCompileScriptJSOK},
    {"testCreateCodeCacheJSOK", testCreateCodeCacheJSOK},
    {"testThrowJSOK", testThrowJSOK},
    {"testOpenHandleScopeJSOK", testOpenHandleScopeJSOK},
    {"testOpenEscapableHandleScopeJSOK", testOpenEscapableHandleScopeJSOK},
    {"testCloseEscapableHandleScopeJSOK", testCloseEscapableHandleScopeJSOK},
    {"testGetReferenceValueJSOK", testGetReferenceValueJSOK},
    {"testCreateArrayJSOK", testCreateArrayJSOK},
    {"testAllocateArrayBufferBackingStoreDataInvalid", testAllocateArrayBufferBackingStoreDataInvalid},
    {"testAllocateArrayBufferBackingStoreDataGenericFailure", testAllocateArrayBufferBackingStoreDataGenericFailure},
    {"testFreeArrayBufferBackingStoreDataInvalid", testFreeArrayBufferBackingStoreDataInvalid},
    {"testCreateArrayBufferFromBackingStoreDataInvalid", testCreateArrayBufferFromBackingStoreDataInvalid},
};

static FunctionDef jsvm_functions2[] = {
    {"testCreateDateJSOK", testCreateDateJSOK},
    {"testCreateExternalJSOK", testCreateExternalJSOK},
    {"testCreateSymbolJSOK", testCreateSymbolJSOK},
    {"testCreateBigintInt64JSOK", testCreateBigintInt64JSOK},
    {"testGetArrayLengthJSOK", testGetArrayLengthJSOK},
    {"testGetPrototypeJSOK", testGetPrototypeJSOK},
    {"testGetDateValueJSOK", testGetDateValueJSOK},
    {"testGetDateValueExpected", testGetDateValueExpected},
    {"testGetValueBoolJSOK", testGetValueBoolJSOK},
    {"testGetValueDoubleExpected", testGetValueDoubleExpected},
    {"testGetValueBigintInt64Expected", testGetValueBigintInt64Expected},
    {"testGetValueBigintUint64JSOK", testGetValueBigintUint64JSOK},
    {"testGetValueBigintUint64Expected", testGetValueBigintUint64Expected},
    {"testGetValueExternalInvalid", testGetValueExternalInvalid},
    {"testGetValueInt32Expected", testGetValueInt32Expected},
    {"testGetValueInt64Expected", testGetValueInt64Expected},
    {"testGetValueStringLatin1JSOK", testGetValueStringLatin1JSOK},
    {"testGetValueStringLatin1Expected", testGetValueStringLatin1Expected},
    {"testGetValueStringUtf8JSOK", testGetValueStringUtf8JSOK},
    {"testGetValueStringUtf8Expected", testGetValueStringUtf8Expected},
    {"testGetValueStringUtf16JSOK", testGetValueStringUtf16JSOK},
    {"testGetValueStringUtf16Expected", testGetValueStringUtf16Expected},
    {"testGetValueUint32JSOK", testGetValueUint32JSOK},
    {"testGetValueUint32Expected", testGetValueUint32Expected},

};

static napi_value RegisterFunctions(napi_env env, napi_value exports, FunctionDef *list, size_t count)
{
    napi_property_descriptor props[count];
    for (size_t i = 0; i < count; i++) {
        props[i] = DECLARE_NAPI_FUNCTION(list[i].name, list[i].cb);
    }
    NAPI_CALL(env, napi_define_properties(env, exports, count, props));
    return nullptr;
}

static napi_value Init(napi_env env, napi_value exports)
{
    RegisterFunctions(env, exports, jsvm_functions1, sizeof(jsvm_functions1) / sizeof(jsvm_functions1[0]));
    RegisterFunctions(env, exports, jsvm_functions2, sizeof(jsvm_functions2) / sizeof(jsvm_functions2[0]));

    return exports;
}

EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&demoModule);
}
