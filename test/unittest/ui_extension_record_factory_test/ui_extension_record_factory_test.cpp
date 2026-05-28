/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "extension_base.h"
#include "ui_extension_record_factory.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "mock_window.h"
#include "mock_permission_verification.h"
#include "mock_bundle_mgr_helper.h"
#include "want.h"

using namespace testing::ext;
using namespace OHOS::Rosen;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {

namespace {
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
}
class UIExtensionRecordFactoryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
};

void UIExtensionRecordFactoryTest::SetUpTestCase(void)
{}

void UIExtensionRecordFactoryTest::TearDownTestCase(void)
{}

void UIExtensionRecordFactoryTest::SetUp()
{}

void UIExtensionRecordFactoryTest::TearDown()
{}

AbilityRequest UIExtensionRecordFactoryTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;
    return abilityRequest;
}

sptr<SessionInfo> UIExtensionRecordFactoryTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    Want want;
    const std::string type = "share";
    want.SetParam("ability.want.params.uiExtensionAbilityId", 1);
    sessionInfo->want = want;
    return sessionInfo;
}

/**
 * @tc.number: NeedReuse_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Set UIEXTENSION_ABILITY_ID  not equal INVALID_EXTENSION_RECORD_ID
 */
HWTEST_F(UIExtensionRecordFactoryTest, NeedReuse_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory info;
    AAFwk::AbilityRequest abilityRequest;
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    auto sessionInfo = MockSessionInfo(1);
    abilityRequest.sessionInfo = sessionInfo;
    int32_t extensionRecordId = 1;
    bool result = info.NeedReuse(abilityRequest, extensionRecordId);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: NeedReuse_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Set UIEXTENSION_ABILITY_ID equal INVALID_EXTENSION_RECORD_ID
 */
HWTEST_F(UIExtensionRecordFactoryTest, NeedReuse_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory info;
    AAFwk::AbilityRequest abilityRequest;
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    auto sessionInfo = MockSessionInfo(1);
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.sessionInfo->want.SetParam(UIEXTENSION_ABILITY_ID, 0);
    int32_t extensionRecordId = 1;
    bool result = info.NeedReuse(abilityRequest, extensionRecordId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: NeedReuse_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Set UIEXTENSION_ABILITY_ID equal INVALID_EXTENSION_RECORD_ID
 */
HWTEST_F(UIExtensionRecordFactoryTest, NeedReuse_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory info;
    AAFwk::AbilityRequest abilityRequest;
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    abilityRequest.sessionInfo = nullptr;
    int32_t extensionRecordId = 1;
    bool result = info.NeedReuse(abilityRequest, extensionRecordId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CreateRecord_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function CreateRecord
 */
HWTEST_F(UIExtensionRecordFactoryTest, CreateRecord_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory info;
    AAFwk::AbilityRequest abilityRequest;
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    auto sessionInfo = MockSessionInfo(1);
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.sessionInfo->want.SetParam(UIEXTENSION_ABILITY_ID, 0);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t result = info.CreateRecord(abilityRequest, extensionRecord);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: CreateRecord_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function CreateRecord
 */
HWTEST_F(UIExtensionRecordFactoryTest, CreateRecord_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory info;
    AAFwk::AbilityRequest abilityRequest;
    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;

    abilityRequest.extensionType == AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    auto sessionInfo = MockSessionInfo(1);
    abilityRequest.sessionInfo = sessionInfo;

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    OHOS::sptr<OHOS::IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    abilityRequest.sessionInfo->callerToken = token;
    auto result = info.CreateRecord(abilityRequest, extensionRecord);
    EXPECT_NE(extensionRecord, nullptr);
    EXPECT_NE(extensionRecord->abilityRecord_, nullptr);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: PreCheck_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function PreCheck
 */
HWTEST_F(UIExtensionRecordFactoryTest, PreCheck_0100, TestSize.Level1)
{
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.ohos.example.hostBundleName";
    abilityRequest.extensionType = ExtensionAbilityType::WORK_SCHEDULER;
    EXPECT_EQ(uiExtensionRecordFactory->PreCheck(abilityRequest, hostBundleName), ERR_OK);
}

/**
 * @tc.number: PreCheck_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function PreCheck
 */
HWTEST_F(UIExtensionRecordFactoryTest, PreCheck_0200, TestSize.Level1)
{
    std::string bundleName = "com.ohos.example.hostBundleName";
    std::string name = "MainAbility";
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.name = name;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.extensionType = static_cast<AppExecFwk::ExtensionAbilityType>(1000);
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    EXPECT_EQ(uiExtensionRecordFactory->PreCheck(abilityRequest, bundleName), ERR_OK);
}

/**
 * @tc.number: PreCheck_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function PreCheck
 */
HWTEST_F(UIExtensionRecordFactoryTest, PreCheck_0300, TestSize.Level1)
{
    std::string hostBundleName = "com.ohos.example.hostBundleName";
    std::string name = "MainAbility";
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = name;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    EXPECT_EQ(uiExtensionRecordFactory->PreCheck(abilityRequest, hostBundleName), ERR_INVALID_VALUE);
}

/**
 * @tc.number: GetExtensionProcessMode_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function GetExtensionProcessMode
 */
HWTEST_F(UIExtensionRecordFactoryTest, GetExtensionProcessMode_0200, TestSize.Level1)
{
    bool isHostSpecified;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    abilityRequest.customProcess = "value";
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    EXPECT_EQ(uiExtensionRecordFactory->GetExtensionProcessMode(abilityRequest, isHostSpecified), PROCESS_MODE_CUSTOM);
}

/**
 * @tc.number: GetExtensionProcessMode_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function GetExtensionProcessMode
 */
HWTEST_F(UIExtensionRecordFactoryTest, GetExtensionProcessMode_0300, TestSize.Level1)
{
    bool isHostSpecified;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    abilityRequest.customProcess = "value";
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    EXPECT_EQ(uiExtensionRecordFactory->GetExtensionProcessMode(abilityRequest, isHostSpecified), PROCESS_MODE_CUSTOM);
}

/**
 * @tc.number: GetExtensionProcessMode_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function GetExtensionProcessMode
 */
HWTEST_F(UIExtensionRecordFactoryTest, GetExtensionProcessMode_0400, TestSize.Level1)
{
    bool isHostSpecified;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.extensionProcessMode = ExtensionProcessMode::UNDEFINED;
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    EXPECT_EQ(uiExtensionRecordFactory->GetExtensionProcessMode(abilityRequest, isHostSpecified), PROCESS_MODE_BUNDLE);
}

/**
 * @tc.number: GetExtensionProcessMode_0500
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function GetExtensionProcessMode
 */
HWTEST_F(UIExtensionRecordFactoryTest, GetExtensionProcessMode_0500, TestSize.Level1)
{
    bool isHostSpecified;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::LIVE_FORM;
    abilityRequest.extensionProcessMode = ExtensionProcessMode::BUNDLE;
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    EXPECT_EQ(uiExtensionRecordFactory->GetExtensionProcessMode(abilityRequest, isHostSpecified), PROCESS_MODE_BUNDLE);
}

/**
 * @tc.number: GetExtensionProcessMode_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function GetExtensionProcessMode
 */
HWTEST_F(UIExtensionRecordFactoryTest, GetExtensionProcessMode_0600, TestSize.Level1)
{
    bool isHostSpecified;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.extensionProcessMode = ExtensionProcessMode::BUNDLE;
    auto uiExtensionRecordFactory = std::make_shared<AbilityRuntime::UIExtensionRecordFactory>();
    EXPECT_EQ(uiExtensionRecordFactory->GetExtensionProcessMode(abilityRequest, isHostSpecified), PROCESS_MODE_BUNDLE);
}

/**
 * @tc.number: CreateDebugRecord_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function CreateDebugRecord
 */
HWTEST_F(UIExtensionRecordFactoryTest, CreateDebugRecord_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory info;
    AAFwk::AbilityRequest abilityRequest;
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    OHOS::sptr<OHOS::IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    abilityRequest.callerToken = token;
    EXPECT_NO_THROW(info.CreateDebugRecord(abilityRequest, abilityRecord));
}

/**
 * @tc.number: CreateDebugRecord_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Call function CreateDebugRecord
 */
HWTEST_F(UIExtensionRecordFactoryTest, CreateDebugRecord_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory info;
    AAFwk::AbilityRequest abilityRequest;
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NO_THROW(info.CreateDebugRecord(abilityRequest, abilityRecord));
}

/**
 * @tc.number: IsInAllowList_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with empty extensionInfos
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with matching extension and caller
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: IsInAllowList_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with allow_all wildcard
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("allow_all");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.any_caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: IsInAllowList_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with non-matching caller
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("com.example.different_caller");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0500
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with wrong extension type
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0500, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;  // Wrong type
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0600, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo2);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0700, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}


/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0800, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}


/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_0900, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}


/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_1000, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller123";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_1100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension88";
    std::string callerAppIdentifier = "com.example.caller123";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_1200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "ServiceExtension";
    std::string callerAppIdentifier = "com.example.caller123";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(UIExtensionRecordFactoryTest, IsInAllowList_1300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "ServiceExtension123";
    std::string callerAppIdentifier = "";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI without permission
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";

    // 设置权限验证返回 false
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and caller app identifier
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    // 设置 Want 中的 caller app identifier
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, callerAppIdentifier);
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(callerAppIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI when not in allow list
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, callerAppIdentifier);
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 不包含 caller 的 allow list
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back("com.example.different_caller");
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}


/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0500, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}


/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0600, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}


/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0700, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}


/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0800, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(false);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0900, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, true);
}


/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1000, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckHostBundleName_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with matching applicationName
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckHostBundleName_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;

    int32_t result = factory.CheckHostBundleName(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: CheckHostBundleName_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with non-matching bundleName
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckHostBundleName_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string appBundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = appBundleName;
    abilityRequest.isTargetPlugin = false;

    // 禁用跨用户权限
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);

    int32_t result = factory.CheckHostBundleName(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: CheckHostBundleName_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with cross-user permission allowed
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckHostBundleName_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    abilityRequest.abilityInfo.applicationName = targetBundleName;
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置权限验证返回 true
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, callerAppIdentifier);

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(callerAppIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    int32_t result = factory.CheckHostBundleName(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: CheckHostBundleName_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with plugin mode
 */
HWTEST_F(UIExtensionRecordFactoryTest, CheckHostBundleName_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";

    abilityRequest.abilityInfo.applicationName = "com.example.different";
    abilityRequest.isTargetPlugin = true;
    abilityRequest.hostBundleName = hostBundleName;

    // 禁用跨用户权限
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);

    int32_t result = factory.CheckHostBundleName(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with SceneBoard
 */
HWTEST_F(UIExtensionRecordFactoryTest, EmbeddedUIPreCheck_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string sceneBoardName = "com.ohos.sceneboard";

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, sceneBoardName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with invalid host bundle name
 */
HWTEST_F(UIExtensionRecordFactoryTest, EmbeddedUIPreCheck_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string appBundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = appBundleName;
    abilityRequest.isTargetPlugin = false;

    // 禁用跨用户权限
    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with valid host and null sessionInfo
 */
HWTEST_F(UIExtensionRecordFactoryTest, EmbeddedUIPreCheck_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;
    abilityRequest.sessionInfo = nullptr;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with non-PAGE caller
 */
HWTEST_F(UIExtensionRecordFactoryTest, EmbeddedUIPreCheck_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;
    abilityRequest.sessionInfo = new (std::nothrow) AAFwk::SessionInfo();

    // 创建一个 SERVICE 类型的 caller ability
    AAFwk::AbilityRequest callerRequest;
    callerRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    callerRequest.abilityInfo.applicationName = bundleName;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(callerRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    abilityRequest.sessionInfo->callerToken = callerToken;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0500
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with valid PAGE caller
 */
HWTEST_F(UIExtensionRecordFactoryTest, EmbeddedUIPreCheck_0500, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;
    abilityRequest.sessionInfo = new (std::nothrow) AAFwk::SessionInfo();

    // 创建一个 PAGE 类型的 caller ability
    AAFwk::AbilityRequest callerRequest;
    callerRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    callerRequest.abilityInfo.applicationName = bundleName;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(callerRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    abilityRequest.sessionInfo->callerToken = callerToken;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with null caller ability record
 */
HWTEST_F(UIExtensionRecordFactoryTest, EmbeddedUIPreCheck_0600, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;
    abilityRequest.sessionInfo = new (std::nothrow) AAFwk::SessionInfo();

    // 设置一个无效的 token
    sptr<IRemoteObject> invalidToken = new (std::nothrow) IRemoteObject();
    abilityRequest.sessionInfo->callerToken = invalidToken;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
}
