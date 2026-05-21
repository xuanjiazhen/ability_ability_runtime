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

#define protected public
#include "extension_record_factory.h"
#undef protected

#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::ExtensionAbilityType;

namespace OHOS {
namespace AAFwk {
class ExtensionRecordFactoryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ExtensionRecordFactoryTest::SetUpTestCase() {}

void ExtensionRecordFactoryTest::TearDownTestCase() {}

void ExtensionRecordFactoryTest::SetUp() {}

void ExtensionRecordFactoryTest::TearDown() {}

/*
 * Feature: PreCheck_0100
 * Function: PreCheck
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, PreCheck_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    char data[] = {"123"};
    int32_t int32Param = 123;
    extensionRecordFactory->NeedReuse(abilityRequest, int32Param);
    std::string strParam("123,1");
    EXPECT_EQ(extensionRecordFactory->PreCheck(abilityRequest, strParam), ERR_OK);
    abilityRequest.extensionType = ExtensionAbilityType::WORK_SCHEDULER;
    EXPECT_EQ(extensionRecordFactory->PreCheck(abilityRequest, strParam), ERR_OK);
    abilityRequest.extensionType = ExtensionAbilityType::INPUTMETHOD;
    EXPECT_EQ(extensionRecordFactory->PreCheck(abilityRequest, strParam), ERR_OK);
}

/*
 * Feature: PreCheck_0200
 * Function: PreCheck
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, PreCheck_0200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    std::string hostBundleName = "com.example.unittest";
    int32_t result = extensionRecordFactory->PreCheck(abilityRequest, "");
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    abilityRequest.abilityInfo.applicationName = "com.example.unittest";
    abilityRequest.sessionInfo =  new (std::nothrow) AAFwk::SessionInfo();
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    abilityRequest.sessionInfo->callerToken = callerToken;
    result = extensionRecordFactory->PreCheck(abilityRequest, "");
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = extensionRecordFactory->PreCheck(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRequest.sessionInfo->callerToken = abilityRecord->GetToken();
    result = extensionRecordFactory->PreCheck(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: PreCheck_0300
 * Function: PreCheck
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, PreCheck_0300, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.unittest";
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.isTargetPlugin = true;
    abilityRequest.hostBundleName = hostBundleName;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    int32_t result = extensionRecordFactory->PreCheck(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: GetExtensionProcessMode_0100
 * Function: GetExtensionProcessMode
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, GetExtensionProcessMode_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest01;
    bool boolParam = false;
    EXPECT_NE(extensionRecordFactory->GetExtensionProcessMode(abilityRequest01, boolParam), ERR_OK);
    boolParam = true;
    EXPECT_NE(extensionRecordFactory->GetExtensionProcessMode(abilityRequest01, boolParam), ERR_OK);
}

/*
 * Feature: GetExtensionProcessMode_0200
 * Function: GetExtensionProcessMode
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, GetExtensionProcessMode_0200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest01;
    abilityRequest01.want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, true);
    bool boolParam = false;
    EXPECT_NE(extensionRecordFactory->GetExtensionProcessMode(abilityRequest01, boolParam), ERR_OK);
    boolParam = true;
    EXPECT_NE(extensionRecordFactory->GetExtensionProcessMode(abilityRequest01, boolParam), ERR_OK);
}

/*
 * Feature: CreateRecord_0100
 * Function: CreateRecord
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, CreateRecord_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRuntime::ExtensionRecord> extensionRecord;
    EXPECT_EQ(extensionRecordFactory->CreateRecord(abilityRequest, extensionRecord), ERR_OK);
}

/*
 * Feature: IsInAllowList_0100
 * Function: IsInAllowList
 * SubFunction: Check with empty extensionInfos list
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_0200
 * Function: IsInAllowList
 * SubFunction: Check with non-EMBEDDED_UI extension type
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    AppExecFwk::ExtensionAbilityInfo info;
    info.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info.name = "TestEmbeddedUI";
    extensionInfos.push_back(info);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_0300
 * Function: IsInAllowList
 * SubFunction: Check with extension name mismatch
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0300, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    AppExecFwk::ExtensionAbilityInfo info;
    info.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info.name = "DifferentEmbeddedUI";
    extensionInfos.push_back(info);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_0400
 * Function: IsInAllowList
 * SubFunction: Check with empty allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0400, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    AppExecFwk::ExtensionAbilityInfo info;
    info.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info.name = "TestEmbeddedUI";
    info.appIdentifierAllowList.clear();
    extensionInfos.push_back(info);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_0500
 * Function: IsInAllowList
 * SubFunction: Check with matching appIdentifier in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0500, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    AppExecFwk::ExtensionAbilityInfo info;
    info.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info.name = "TestEmbeddedUI";
    info.appIdentifierAllowList.push_back("com.caller.app");
    info.appIdentifierAllowList.push_back("com.other.app");
    extensionInfos.push_back(info);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/*
 * Feature: IsInAllowList_0600
 * Function: IsInAllowList
 * SubFunction: Check with allow_all in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0600, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    AppExecFwk::ExtensionAbilityInfo info;
    info.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info.name = "TestEmbeddedUI";
    info.appIdentifierAllowList.push_back("allow_all");
    extensionInfos.push_back(info);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/*
 * Feature: IsInAllowList_0700
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0700, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/*
 * Feature: IsInAllowList_0800
 * Function: IsInAllowList
 * SubFunction: Check with appIdentifier not in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0800, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    AppExecFwk::ExtensionAbilityInfo info;
    info.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info.name = "TestEmbeddedUI";
    info.appIdentifierAllowList.push_back("com.other.app");
    info.appIdentifierAllowList.push_back("com.another.app");
    extensionInfos.push_back(info);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_0800
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0800, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info2.name = "TestEmbeddedUI1";
    info2.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_0800
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_0900, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info2.name = "TestEmbeddedUI1";
    info2.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI2";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1000, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1200
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI123";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1300, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI321";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1400, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI888";
    std::string callerAppIdentifier = "com.caller.app888";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1500, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1600, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app888");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI888";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI888";
    std::string callerAppIdentifier = "com.caller.app888";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1700, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension999";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI999";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI999";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI999";
    std::string callerAppIdentifier = "com.caller.app999";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1800, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    info3.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info3);

    std::string extensionName = "AnotherEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_1900, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    info3.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info3);

    std::string extensionName = "AnotherEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app666";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_2000, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI444";
    info3.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info3);

    std::string extensionName = "AnotherEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_2100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info3.name = "AnotherEmbeddedUI";
    info3.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info3);

    std::string extensionName = "AnotherEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_2200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    info3.appIdentifierAllowList.push_back("com.caller.app555");
    extensionInfos.push_back(info3);

    std::string extensionName = "AnotherEmbeddedUI";
    std::string callerAppIdentifier = "com.caller.app555";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_2300, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI333";
    info3.appIdentifierAllowList.push_back("com.caller.app");
    extensionInfos.push_back(info3);

    std::string extensionName = "AnotherEmbeddedUI333";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_2400, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "";
    std::string callerAppIdentifier = "com.caller.app";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: IsInAllowList_1000
 * Function: IsInAllowList
 * SubFunction: Check with multiple extensions, find correct one
 */
HWTEST_F(ExtensionRecordFactoryTest, IsInAllowList_2500, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo info1;
    info1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info1.name = "ServiceExtension";
    extensionInfos.push_back(info1);

    AppExecFwk::ExtensionAbilityInfo info2;
    info2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    info2.name = "TestEmbeddedUI";
    info2.appIdentifierAllowList.push_back("com.caller.app111");
    extensionInfos.push_back(info2);

    AppExecFwk::ExtensionAbilityInfo info3;
    info3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    info3.name = "AnotherEmbeddedUI";
    extensionInfos.push_back(info3);

    std::string extensionName = "TestEmbeddedUI";
    std::string callerAppIdentifier = "";

    bool result = extensionRecordFactory->IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0100
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with permission verification failed
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    std::string hostBundleName = "com.host.app";

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);
}

/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, true);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}

/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0300
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller not in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0300, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.other.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}

/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0400
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with allow_all in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0400, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock signature info
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("allow_all");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockSignatureInfo(signatureInfo);
        mockInstance->SetGetSignatureInfoResult(ERR_OK);
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, true);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}

/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0500
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with GetBundleInfo failed
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0500, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock signature info
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = "com.host.app";

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockSignatureInfo(signatureInfo);
        mockInstance->SetGetSignatureInfoResult(ERR_OK);
        mockInstance->SetGetBundleInfoResult(false);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}

/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0600
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with GetSignatureInfoByBundleName failed
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0600, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetGetSignatureInfoResult(ERR_INVALID_VALUE);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}

/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0700
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with extension type not EMBEDDED_UI
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0700, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestService";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extInfo.name = "TestService";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0800, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app111");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_0900, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1000, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI111";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI444";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1300, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1400, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1500, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, "com.caller.app");
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}


/*
 * Feature: CheckAllowCrossUserEmbeddedUI_0200
 * Function: CheckAllowCrossUserEmbeddedUI
 * SubFunction: Check with caller in allow list
 */
HWTEST_F(ExtensionRecordFactoryTest, CheckAllowCrossUserEmbeddedUI_1600, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(true);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.target.app";
    abilityRequest.abilityInfo.name = "TestEmbeddedUI";
    abilityRequest.userId = 100;
    std::string hostBundleName = "com.host.app";

    // Prepare mock bundle info
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extInfo;
    extInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extInfo.name = "TestEmbeddedUI";
    extInfo.appIdentifierAllowList.push_back("com.caller.app");
    bundleInfo.extensionInfos.push_back(extInfo);

    auto mockInstance = static_cast<AppExecFwk::AbilityManagerMock*>(
        AppExecFwk::BundleMgrHelper::GetBundleMgrHelper().GetRefPtr());
    if (mockInstance) {
        mockInstance->SetMockBundleInfo(bundleInfo);
        mockInstance->SetGetBundleInfoResult(true);
    }

    bool result = extensionRecordFactory->CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);

    AAFwk::PermissionVerification::GetInstance()->SetVerifySupportCrossAppEmbedForOaPermission(false);
}
} // namespace AAFwk
} // namespace OHOS
