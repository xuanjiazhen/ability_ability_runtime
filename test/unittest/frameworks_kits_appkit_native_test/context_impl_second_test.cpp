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
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "res_config.h"
#include "res_config_impl.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const int64_t CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
} // namespace

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

bool CheckContext(const std::shared_ptr<AbilityRuntime::Context> inputContext,
    std::string &bundleName, std::string &moduleName)
{
    if (inputContext == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "Context is error");
        return false;
    }

    if (inputContext->GetHapModuleInfo() == nullptr) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
        resourceManager = inputContext->GetResourceManager();
        if (resourceManager != nullptr) {
            moduleName = resourceManager->bundleInfo.second;
        }
    } else {
        moduleName = inputContext->GetHapModuleInfo()->moduleName;
    }
    bundleName = inputContext->GetBundleName();
    return true;
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateHspResourceManager_001
 * @tc.name: CreateHspResourceManager
 * @tc.desc: Test whether CreateHspResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateHspResourceManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_002 start";

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::AbilityStageContext> abilityStageContext = nullptr;
    abilityStageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "com.clm.module";
    bundleInfo.applicationInfo.name = "applicationName";
    bundleInfo.isKeepAlive = true;
    bundleInfo.applicationInfo.process = "com.li.module";
    bundleInfo.applicationInfo.multiProjects = true;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    std::shared_ptr<AbilityRuntime::ContextImpl> context = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = "com.module";
    context->SetApplicationInfo(applicationInfo);
    context->InitHapModuleInfo(hapModuleInfo);
    contextImpl_->InitResourceManager(bundleInfo, context, true, "test_moduleName");

    abilityStageContext->SetParentContext(context);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    Global::Resource::ResConfigImpl resConfig;
    resourceManager = abilityStageContext->GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(resConfig);
        resConfig.SetMcc(11);
        resConfig.SetMnc(22);
        resConfig.SetThemeId(33);
        resourceManager->UpdateResConfig(resConfig);
    }

    contextImpl_->GetBundleManager();
    sptr<IRemoteObject> bundleService = new (std::nothrow) BundleMgrService();
    if (bundleService != nullptr) {
        contextImpl_->bundleMgr_->bundleMgr_ = iface_cast<IBundleMgr>(bundleService);
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager2 = nullptr;
    contextImpl_->CreateHspModuleResourceManager(abilityStageContext, resourceManager2);
    Global::Resource::ResConfigImpl resConfig2;
    resourceManager2->GetResConfig(resConfig2);
    EXPECT_EQ(resConfig2.GetMcc(), 11);
    EXPECT_EQ(resConfig2.GetMnc(), 22);
    EXPECT_EQ(resConfig2.GetThemeId(), 33);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_002 end";
}
}  // namespace AppExecFwk
}
