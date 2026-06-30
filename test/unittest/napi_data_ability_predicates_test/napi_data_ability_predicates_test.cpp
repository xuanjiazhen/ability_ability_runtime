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

#include <gtest/gtest.h>

#include <memory>
#include <new>
#include <string>

#include "napi_data_ability_predicates.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"

using namespace testing::ext;

namespace OHOS {
namespace DataAbilityJsKit {
namespace {
napi_env g_env = nullptr;
panda::ecmascript::EcmaVM *g_vm = nullptr;
}

class NapiDataAbilityPredicatesTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        panda::RuntimeOption option;
        option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
        option.SetGcPoolSize(0x1000000);
        option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
        option.SetDebuggerLibraryPath("");
        g_vm = panda::JSNApi::CreateJSVM(option);
        ASSERT_NE(g_vm, nullptr);
        g_env = reinterpret_cast<napi_env>(new (std::nothrow) ArkNativeEngine(g_vm, nullptr));
        ASSERT_NE(g_env, nullptr);

        napi_value exports = nullptr;
        ASSERT_EQ(napi_create_object(g_env, &exports), napi_ok);
        DataAbilityPredicatesProxy::Init(g_env, exports);
    }

    static void TearDownTestCase()
    {
        delete reinterpret_cast<ArkNativeEngine *>(g_env);
        g_env = nullptr;
        if (g_vm != nullptr) {
            panda::JSNApi::DestroyJSVM(g_vm);
            g_vm = nullptr;
        }
    }
};

/**
 * @tc.name: SetOrder_ValidString_001
 * @tc.desc: A valid string updates the native predicate order.
 * @tc.type: FUNC
 */
HWTEST_F(NapiDataAbilityPredicatesTest, SetOrder_ValidString_001, TestSize.Level1)
{
    HandleScope handleScope(g_env);
    auto predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    napi_value instance = DataAbilityPredicatesProxy::NewInstance(g_env, predicates);
    ASSERT_NE(instance, nullptr);
    napi_value order = nullptr;
    ASSERT_EQ(napi_create_string_utf8(g_env, "name ASC", NAPI_AUTO_LENGTH, &order), napi_ok);

    EXPECT_EQ(napi_set_named_property(g_env, instance, "order", order), napi_ok);
    EXPECT_EQ(predicates->GetOrder(), "name ASC");
}

/**
 * @tc.name: SetOrder_InvalidType_001
 * @tc.desc: A non-string value is rejected and does not overwrite the current order.
 * @tc.type: FUNC
 */
HWTEST_F(NapiDataAbilityPredicatesTest, SetOrder_InvalidType_001, TestSize.Level1)
{
    HandleScope handleScope(g_env);
    auto predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    predicates->SetOrder("name DESC");
    napi_value instance = DataAbilityPredicatesProxy::NewInstance(g_env, predicates);
    ASSERT_NE(instance, nullptr);
    napi_value invalidOrder = nullptr;
    ASSERT_EQ(napi_create_int32(g_env, 1, &invalidOrder), napi_ok);

    (void)napi_set_named_property(g_env, instance, "order", invalidOrder);
    EXPECT_EQ(predicates->GetOrder(), "name DESC");
}
} // namespace DataAbilityJsKit
} // namespace OHOS
