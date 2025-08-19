/*
* Copyright (c) 2025 SwanLink (Jiangsu) Technology Development Co., LTD.
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

#ifndef ARK_RUNTIME_JSVM_TEST_NAPI_ERROR_TEST_H
#define ARK_RUNTIME_JSVM_TEST_NAPI_ERROR_TEST_H

#include <napi/native_api.h>

napi_value testCreateVMJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateProxyJSOK(napi_env env1, napi_callback_info info);
napi_value testCreateProxyPending(napi_env env1, napi_callback_info info);
napi_value testCreateProxyInvalid(napi_env env1, napi_callback_info info);
napi_value testCreateProxyObejct(napi_env env1, napi_callback_info info);

napi_value testIsProxyJSOK(napi_env env1, napi_callback_info info);
napi_value testIsProxyInvalid(napi_env env1, napi_callback_info info);

napi_value testProxyGetTargetJSOK(napi_env env1, napi_callback_info info);
napi_value testProxyGetTargetInvalid(napi_env env1, napi_callback_info info);
napi_value testProxyGetTargetInvalidType(napi_env env1, napi_callback_info info);

napi_value testOpenVMScopeJSOK(napi_env env1, napi_callback_info info);

napi_value testCloseVMScopeJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateEnvJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateEnvFromSnapshotJSOK(napi_env env1, napi_callback_info info);

napi_value testDestroyEnvJSOK(napi_env env1, napi_callback_info info);

napi_value testOpenEnvScopeJSOK(napi_env env1, napi_callback_info info);

napi_value testCloseEnvScopeJSOK(napi_env env1, napi_callback_info info);

napi_value testGetVMJSOK(napi_env env1, napi_callback_info info);

napi_value testCompileScriptJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateCodeCacheJSOK(napi_env env1, napi_callback_info info);

napi_value testThrowJSOK(napi_env env1, napi_callback_info info);

napi_value testOpenHandleScopeJSOK(napi_env env1, napi_callback_info info);

napi_value testOpenEscapableHandleScopeJSOK(napi_env env1, napi_callback_info info);

napi_value testCloseEscapableHandleScopeJSOK(napi_env env1, napi_callback_info info);

napi_value testGetReferenceValueJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateArrayJSOK(napi_env env1, napi_callback_info info);

napi_value testAllocateArrayBufferBackingStoreDataInvalid(napi_env env1, napi_callback_info info);

napi_value testAllocateArrayBufferBackingStoreDataGenericFailure(napi_env env1, napi_callback_info info);

napi_value testFreeArrayBufferBackingStoreDataInvalid(napi_env env1, napi_callback_info info);

napi_value testCreateArrayBufferFromBackingStoreDataInvalid(napi_env env1, napi_callback_info info);

napi_value testCreateDateJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateExternalJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateSymbolJSOK(napi_env env1, napi_callback_info info);

napi_value testCreateBigintInt64JSOK(napi_env env1, napi_callback_info info);

napi_value testGetArrayLengthJSOK(napi_env env1, napi_callback_info info);

napi_value testGetPrototypeJSOK(napi_env env1, napi_callback_info info);

napi_value testGetDateValueJSOK(napi_env env1, napi_callback_info info);
napi_value testGetDateValueExpected(napi_env env1, napi_callback_info info);

napi_value testGetValueBoolJSOK(napi_env env1, napi_callback_info info);

napi_value testGetValueDoubleExpected(napi_env env1, napi_callback_info info);

napi_value testGetValueBigintInt64Expected(napi_env env1, napi_callback_info info);

napi_value testGetValueBigintUint64JSOK(napi_env env1, napi_callback_info info);
napi_value testGetValueBigintUint64Expected(napi_env env1, napi_callback_info info);

napi_value testGetValueExternalInvalid(napi_env env1, napi_callback_info info);

napi_value testGetValueInt32Expected(napi_env env1, napi_callback_info info);

napi_value testGetValueInt64Expected(napi_env env1, napi_callback_info info);

napi_value testGetValueStringLatin1JSOK(napi_env env1, napi_callback_info info);
napi_value testGetValueStringLatin1Expected(napi_env env1, napi_callback_info info);

napi_value testGetValueStringUtf8JSOK(napi_env env1, napi_callback_info info);
napi_value testGetValueStringUtf8Expected(napi_env env1, napi_callback_info info);

napi_value testGetValueStringUtf16JSOK(napi_env env1, napi_callback_info info);
napi_value testGetValueStringUtf16Expected(napi_env env1, napi_callback_info info);

napi_value testGetValueUint32JSOK(napi_env env1, napi_callback_info info);
napi_value testGetValueUint32Expected(napi_env env1, napi_callback_info info);

#endif //ARK_RUNTIME_JSVM_TEST_NAPI_ERROR_TEST_H
