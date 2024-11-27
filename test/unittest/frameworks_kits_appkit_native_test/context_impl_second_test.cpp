/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <singleton.h>

#define private public
#include "context_impl.h"
#include "bundle_mgr_helper.h"
#undef private

#include "constants.h"
#include "ability_local_record.h"
#include "application_context.h"
#include "ability_stage_context.h"
#include "context.h"
#include "hap_module_info.h"
#include "iremote_object.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "res_config_impl.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ContextImplSecondTest : public testing::Test {
public:
    ContextImplSecondTest()
    {}
    ~ContextImplSecondTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextImplSecondTest::SetUpTestCase(void)
{}

void ContextImplSecondTest::TearDownTestCase(void)
{}

void ContextImplSecondTest::SetUp(void)
{}

void ContextImplSecondTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_AppContext_CreateHspResourceManager_001
 * @tc.name: CreateHspResourceManager
 * @tc.desc: Test whether CreateHspResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateHspResourceManager_001, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl_, nullptr);
}
}  // namespace AppExecFwk
}
