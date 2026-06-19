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

#include <cstdint>
#include <gtest/gtest.h>
#include <vector>

#include "ecmascript/napi/include/jsnapi.h"
#include "js_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppDataMgrJsKit;

class NapiJsUtilsTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    static napi_env env_;
    static panda::ecmascript::EcmaVM* vm_;
    static NativeEngine* engine_;
};

napi_env NapiJsUtilsTest::env_ = nullptr;
panda::ecmascript::EcmaVM* NapiJsUtilsTest::vm_ = nullptr;
NativeEngine* NapiJsUtilsTest::engine_ = nullptr;

void NapiJsUtilsTest::SetUpTestCase()
{
    panda::RuntimeOption pandaOption;
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    ASSERT_NE(vm_, nullptr);
    engine_ = new ArkNativeEngine(vm_, nullptr);
    env_ = reinterpret_cast<napi_env>(engine_);
    ASSERT_NE(env_, nullptr);
}

void NapiJsUtilsTest::TearDownTestCase()
{
    if (engine_ != nullptr) {
        delete engine_;
        engine_ = nullptr;
    }
    if (vm_ != nullptr) {
        panda::JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
    env_ = nullptr;
}

static napi_value CreateFloat32Array(napi_env env, size_t byteLength)
{
    void *native = nullptr;
    napi_value buffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, byteLength, &native, &buffer);
    if (status != napi_ok || native == nullptr) {
        return nullptr;
    }
    float *floatPtr = static_cast<float *>(native);
    size_t elementCount = byteLength / sizeof(float);
    for (size_t i = 0; i < elementCount; i++) {
        floatPtr[i] = static_cast<float>(i);
    }
    napi_value typedArray = nullptr;
    status = napi_create_typedarray(env, napi_float32_array, elementCount, buffer, 0, &typedArray);
    if (status != napi_ok) {
        return nullptr;
    }
    return typedArray;
}

static napi_value CreateUint8Array(napi_env env, size_t byteLength)
{
    void *native = nullptr;
    napi_value buffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, byteLength, &native, &buffer);
    if (status != napi_ok || native == nullptr) {
        return nullptr;
    }
    uint8_t *uintPtr = static_cast<uint8_t *>(native);
    for (size_t i = 0; i < byteLength; i++) {
        uintPtr[i] = static_cast<uint8_t>(i % UINT8_MAX + 1);
    }
    napi_value typedArray = nullptr;
    status = napi_create_typedarray(env, napi_uint8_array, byteLength, buffer, 0, &typedArray);
    if (status != napi_ok) {
        return nullptr;
    }
    return typedArray;
}

/**
 * @tc.name: Convert2Value_VectorFloat_NormalSize
 * @tc.desc: Verify that Convert2Value for vector<float> correctly converts
 *           a Float32Array with normal size (below MAX_VALUE_LENGTH) without truncation.
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorFloat_NormalSize, TestSize.Level1)
{
    napi_value jsValue = CreateFloat32Array(env_, 1024);
    ASSERT_NE(jsValue, nullptr);
    std::vector<float> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsValue, output);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(output.size(), 1024 / sizeof(float));
}

/**
 * @tc.name: Convert2Value_VectorFloat_Truncated
 * @tc.desc: Verify that Convert2Value for vector<float> truncates the output
 *           when the Float32Array byte length exceeds MAX_VALUE_LENGTH,
 *           and the truncated length satisfies length % sizeof(float) == 0
 *           to prevent partial float element reads.
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorFloat_Truncated, TestSize.Level1)
{
    size_t overLength = JSUtils::MAX_VALUE_LENGTH + 16;
    napi_value jsValue = CreateFloat32Array(env_, overLength);
    if (jsValue == nullptr) {
        GTEST_LOG_(INFO) << "Skip: cannot allocate oversized typedarray";
        return;
    }
    std::vector<float> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsValue, output);
    EXPECT_EQ(ret, napi_ok);
    size_t truncatedByteLen = (JSUtils::MAX_VALUE_LENGTH / sizeof(float)) * sizeof(float);
    EXPECT_EQ(output.size(), truncatedByteLen / sizeof(float));
    EXPECT_EQ(output.size() * sizeof(float) % sizeof(float), 0);
}

/**
 * @tc.name: Convert2Value_VectorUint8_NormalSize
 * @tc.desc: Verify that Convert2Value for vector<uint8_t> correctly converts
 *           a Uint8Array with normal size (below MAX_VALUE_LENGTH) without truncation.
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorUint8_NormalSize, TestSize.Level1)
{
    napi_value jsValue = CreateUint8Array(env_, 1024);
    ASSERT_NE(jsValue, nullptr);
    std::vector<uint8_t> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsValue, output);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(output.size(), 1024);
}

/**
 * @tc.name: Convert2Value_VectorUint8_Truncated
 * @tc.desc: Verify that Convert2Value for vector<uint8_t> truncates the output
 *           to MAX_VALUE_LENGTH when the Uint8Array length exceeds MAX_VALUE_LENGTH.
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorUint8_Truncated, TestSize.Level1)
{
    size_t overLength = JSUtils::MAX_VALUE_LENGTH + 1;
    napi_value jsValue = CreateUint8Array(env_, overLength);
    if (jsValue == nullptr) {
        GTEST_LOG_(INFO) << "Skip: cannot allocate oversized typedarray";
        return;
    }
    std::vector<uint8_t> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsValue, output);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(output.size(), JSUtils::MAX_VALUE_LENGTH);
}

/**
 * @tc.name: Convert2Value_VectorFloat_NotTypedArray
 * @tc.desc: Verify that Convert2Value for vector<float> returns napi_invalid_arg
 *           when the input is not a typed array (e.g., a plain JS object).
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorFloat_NotTypedArray, TestSize.Level1)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env_, &jsObj);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(jsObj, nullptr);
    std::vector<float> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsObj, output);
    EXPECT_EQ(ret, napi_invalid_arg);
}

/**
 * @tc.name: Convert2Value_VectorUint8_NotTypedArray
 * @tc.desc: Verify that Convert2Value for vector<uint8_t> returns napi_invalid_arg
 *           when the input is not a typed array (e.g., a plain JS object).
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorUint8_NotTypedArray, TestSize.Level1)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env_, &jsObj);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(jsObj, nullptr);
    std::vector<uint8_t> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsObj, output);
    EXPECT_EQ(ret, napi_invalid_arg);
}

/**
 * @tc.name: Convert2Value_VectorFloat_WrongTypedArrayType
 * @tc.desc: Verify that Convert2Value for vector<float> returns napi_invalid_arg
 *           when the input typed array has a wrong type (Uint8Array instead of Float32Array).
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorFloat_WrongTypedArrayType, TestSize.Level1)
{
    napi_value jsValue = CreateUint8Array(env_, 64);
    ASSERT_NE(jsValue, nullptr);
    std::vector<float> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsValue, output);
    EXPECT_EQ(ret, napi_invalid_arg);
}

/**
 * @tc.name: Convert2Value_VectorUint8_WrongTypedArrayType
 * @tc.desc: Verify that Convert2Value for vector<uint8_t> returns napi_invalid_arg
 *           when the input typed array has a wrong type (Float32Array instead of Uint8Array).
 */
HWTEST_F(NapiJsUtilsTest, Convert2Value_VectorUint8_WrongTypedArrayType, TestSize.Level1)
{
    napi_value jsValue = CreateFloat32Array(env_, 64);
    ASSERT_NE(jsValue, nullptr);
    std::vector<uint8_t> output;
    int32_t ret = JSUtils::Convert2Value(env_, jsValue, output);
    EXPECT_EQ(ret, napi_invalid_arg);
}
