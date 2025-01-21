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

#include <iostream>
#include <string>
#include <gtest/gtest.h>
#define private public
#define protected public
#include "cj_ability_monitor_object.h"
#include "cj_ability_monitor_object.cpp"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityDelegatorCJ  {
class CjAbilityMonitorObjectTest : public testing::Test {
public:
    CjAbilityMonitorObjectTest()
    {}
    ~CjAbilityMonitorObjectTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
protected:
    std::shared_ptr<CJMonitorObject> cjMonitorObj_;
};

void CjAbilityMonitorObjectTest::SetUpTestCase()
{}

void CjAbilityMonitorObjectTest::TearDownTestCase()
{}

void CjAbilityMonitorObjectTest::SetUp()
{
    int64_t monitorId = 1;
    cjMonitorObj_ = std::make_shared<CJMonitorObject>(monitorId);
}

void CjAbilityMonitorObjectTest::TearDown()
{}

/**
 * @tc.name: CjAbilityMonitorObjectTestOnAbilityCreate_0100
 * @tc.desc: CjAbilityMonitorObjectTest test for OnAbilityCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityMonitorObjectTest, CjAbilityMonitorObjectTestOnAbilityCreate_0100, TestSize.Level1)
{
    int64_t abilityId = 1;
    cjMonitorObj_->OnAbilityCreate(abilityId);
    EXPECT_EQ(g_cjMonitorFuncs.cjOnAbilityCreate, nullptr);

    g_cjMonitorFuncs.cjOnAbilityCreate = [](int64_t monitorId, int64_t abilityId) {};
    cjMonitorObj_->OnAbilityCreate(abilityId);
    EXPECT_NE(g_cjMonitorFuncs.cjOnAbilityCreate, nullptr);
}

/**
 * @tc.name: CjAbilityMonitorObjectTestOnAbilityForeground_0100
 * @tc.desc: CjAbilityMonitorObjectTest test for OnAbilityForeground.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityMonitorObjectTest, CjAbilityMonitorObjectTestOnAbilityForeground_0100, TestSize.Level1)
{
    int64_t abilityId = 1;
    cjMonitorObj_->OnAbilityForeground(abilityId);
    EXPECT_EQ(g_cjMonitorFuncs.cjOnAbilityForeground, nullptr);

    g_cjMonitorFuncs.cjOnAbilityForeground = [](int64_t monitorId, int64_t abilityId) {};
    cjMonitorObj_->OnAbilityForeground(abilityId);
    EXPECT_NE(g_cjMonitorFuncs.cjOnAbilityForeground, nullptr);
}

/**
 * @tc.name: CjAbilityMonitorObjectTestOnAbilityBackground_0100
 * @tc.desc: CjAbilityMonitorObjectTest test for OnAbilityBackground.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityMonitorObjectTest, CjAbilityMonitorObjectTestOnAbilityBackground_0100, TestSize.Level1)
{
    int64_t abilityId = 1;
    cjMonitorObj_->OnAbilityBackground(abilityId);
    EXPECT_EQ(g_cjMonitorFuncs.cjOnAbilityBackground, nullptr);

    g_cjMonitorFuncs.cjOnAbilityBackground = [](int64_t monitorId, int64_t abilityId) {};
    cjMonitorObj_->OnAbilityBackground(abilityId);
    EXPECT_NE(g_cjMonitorFuncs.cjOnAbilityBackground, nullptr);
}

/**
 * @tc.name: CjAbilityMonitorObjectTestOnAbilityDestroy_0100
 * @tc.desc: CjAbilityMonitorObjectTest test for OnAbilityDestroy.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityMonitorObjectTest, CjAbilityMonitorObjectTestOnAbilityDestroy_0100, TestSize.Level1)
{
    int64_t abilityId = 1;
    cjMonitorObj_->OnAbilityDestroy(abilityId);
    EXPECT_EQ(g_cjMonitorFuncs.cjOnAbilityDestroy, nullptr);

    g_cjMonitorFuncs.cjOnAbilityDestroy = [](int64_t monitorId, int64_t abilityId) {};
    cjMonitorObj_->OnAbilityDestroy(abilityId);
    EXPECT_NE(g_cjMonitorFuncs.cjOnAbilityDestroy, nullptr);
}

/**
 * @tc.name: CjAbilityMonitorObjectTestOnWindowStageCreate_0100
 * @tc.desc: CjAbilityMonitorObjectTest test for OnWindowStageCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityMonitorObjectTest, CjAbilityMonitorObjectTestOnWindowStageCreate_0100, TestSize.Level1)
{
    int64_t abilityId = 1;
    cjMonitorObj_->OnWindowStageCreate(abilityId);
    EXPECT_EQ(g_cjMonitorFuncs.cjOnWindowStageCreate, nullptr);

    g_cjMonitorFuncs.cjOnWindowStageCreate = [](int64_t monitorId, int64_t abilityId) {};
    cjMonitorObj_->OnWindowStageCreate(abilityId);
    EXPECT_NE(g_cjMonitorFuncs.cjOnWindowStageCreate, nullptr);
}

/**
 * @tc.name: CjAbilityMonitorObjectTestOnWindowStageRestore_0100
 * @tc.desc: CjAbilityMonitorObjectTest test for OnWindowStageRestore.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityMonitorObjectTest, CjAbilityMonitorObjectTestOnWindowStageRestore_0100, TestSize.Level1)
{
    int64_t abilityId = 1;
    cjMonitorObj_->OnWindowStageRestore(abilityId);
    EXPECT_EQ(g_cjMonitorFuncs.cjOnWindowStageRestore, nullptr);

    g_cjMonitorFuncs.cjOnWindowStageRestore = [](int64_t monitorId, int64_t abilityId) {};
    cjMonitorObj_->OnWindowStageRestore(abilityId);
    EXPECT_NE(g_cjMonitorFuncs.cjOnWindowStageRestore, nullptr);
}

/**
 * @tc.name: CjAbilityMonitorObjectTestOnWindowStageDestroy_0100
 * @tc.desc: CjAbilityMonitorObjectTest test for OnWindowStageDestroy.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityMonitorObjectTest, CjAbilityMonitorObjectTestOnWindowStageDestroy_0100, TestSize.Level1)
{
    int64_t abilityId = 1;
    cjMonitorObj_->OnWindowStageDestroy(abilityId);
    EXPECT_EQ(g_cjMonitorFuncs.cjOnWindowStageDestroy, nullptr);

    g_cjMonitorFuncs.cjOnWindowStageDestroy = [](int64_t monitorId, int64_t abilityId) {};
    cjMonitorObj_->OnWindowStageDestroy(abilityId);
    EXPECT_NE(g_cjMonitorFuncs.cjOnWindowStageDestroy, nullptr);
}
}  // namespace AbilityRuntime
}  // namespace OHOS