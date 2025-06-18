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
#include "parcel.h"

#define private public
#include "ui_extension/ui_extension_session_info.h"
#include "hilog_tag_wrapper.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionSessionInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionSessionInfoTest::SetUpTestCase(void)
{}

void UIExtensionSessionInfoTest::TearDownTestCase(void)
{}

void UIExtensionSessionInfoTest::SetUp()
{}

void UIExtensionSessionInfoTest::TearDown()
{}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Marshalling_0100, TestSize.Level1)
{
    Parcel parcel;
    UIExtensionSessionInfo* parcelable1 = new UIExtensionSessionInfo();
    parcelable1->persistentId = 1;
    parcelable1->hostWindowId = 1;
    parcelable1->uiExtensionUsage = AAFwk::UIExtensionUsage::MODAL;
    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    AppExecFwk::ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    parcelable1->elementName = elementName;
    parcelable1->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string hostDeviceId;
    std::string hostBundleName = "ohos.test.bundle";
    std::string hostAbilityName = "TestAbility";
    std::string hostModuleName = "entry";
    AppExecFwk::ElementName hostElementName;
    hostElementName.SetDeviceID(deviceId);
    hostElementName.SetBundleName(hostBundleName);
    hostElementName.SetAbilityName(hostAbilityName);
    hostElementName.SetModuleName(hostModuleName);
    parcelable1->hostElementName = hostElementName;
    EXPECT_EQ(true, parcelable1->Marshalling(parcel));
    UIExtensionSessionInfo* parcelable2 = parcelable1->Unmarshalling(parcel);
    EXPECT_EQ(parcelable2->persistentId, 1);
    EXPECT_EQ(parcelable2->hostWindowId, 1);
    EXPECT_EQ(parcelable2->uiExtensionUsage, AAFwk::UIExtensionUsage::MODAL);
    EXPECT_EQ(parcelable2->elementName, elementName);
    EXPECT_EQ(parcelable2->extensionAbilityType, AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON);
    EXPECT_EQ(parcelable2->hostElementName, hostElementName);
}
} // namespace AbilityRuntime
} // namespace OHOS