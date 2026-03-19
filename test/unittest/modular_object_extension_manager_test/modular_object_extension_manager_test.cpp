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

#include <gtest/gtest.h>

#include "modular_object_extension_manager.h"

#include <cstring>
#include "ability_manager/include/modular_object_extension_info.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {

class AbilityRuntimeModularObjectExtensionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityRuntimeModularObjectExtensionManagerTest::SetUpTestCase()
{}

void AbilityRuntimeModularObjectExtensionManagerTest::TearDownTestCase()
{}

void AbilityRuntimeModularObjectExtensionManagerTest::SetUp()
{}

void AbilityRuntimeModularObjectExtensionManagerTest::TearDown()
{}

/**
 * @tc.name: OH_AbilityRuntime_DestroyExtensionInfo_001
 * @tc.desc: OH_AbilityRuntime_DestroyExtensionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest, OH_AbilityRuntime_DestroyExtensionInfo_001, TestSize.Level1)
{

}
}  // namespace AAFwk
}  // namespace OHOS
