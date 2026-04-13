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

#include "ui_ability_record.h"
#include "native_ability_util.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class UIAbilityRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIAbilityRecordTest::SetUpTestCase(void)
{}
void UIAbilityRecordTest::TearDownTestCase(void)
{}
void UIAbilityRecordTest::SetUp()
{}
void UIAbilityRecordTest::TearDown()
{}

/**
 * @tc.name: ScheduleCollaborate_0010
 * @tc.desc: lifecycleDeal null or not.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, ScheduleCollaborate_0010, TestSize.Level2)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->ScheduleCollaborate(abilityRequest.want);

    auto lifecycleDeal = std::make_shared<LifecycleDeal>();
    abilityRecord->lifecycleDeal_ = lifecycleDeal;
    EXPECT_CALL(*lifecycleDeal, ScheduleCollaborate).Times(1);
    abilityRecord->ScheduleCollaborate(abilityRequest.want);
}

/**
 * @tc.name: CreateAbilityRecord_0100
 * @tc.desc: No metadata, abilityNativeState should be NONE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    // No metadata configured, default state
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);
}

/**
 * @tc.name: CreateAbilityRecord_0200
 * @tc.desc: withNativeModule=true, startupPhase=PRE_WINDOW, abilityNativeState should be INIT.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0200, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    AppExecFwk::Metadata metaWithNative;
    metaWithNative.name = "ohos.ability.withNativeModule";
    metaWithNative.value = "true";
    abilityRequest.abilityInfo.metadata.push_back(metaWithNative);

    AppExecFwk::Metadata metaSource;
    metaSource.name = "ohos.ability.nativeModuleSource";
    metaSource.value = "libtest.so";
    abilityRequest.abilityInfo.metadata.push_back(metaSource);

    AppExecFwk::Metadata metaFunc;
    metaFunc.name = "ohos.ability.nativeModuleFun";
    metaFunc.value = "OHMain";
    abilityRequest.abilityInfo.metadata.push_back(metaFunc);

    // startupPhase defaults to PRE_WINDOW when not specified
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);
}

/**
 * @tc.name: CreateAbilityRecord_0300
 * @tc.desc: withNativeModule=true, startupPhase=PRE_FOREGROUND, abilityNativeState should be INIT.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0300, TestSize.Level1)
{
    AbilityRequest abilityRequest;

    AppExecFwk::Metadata metaWithNative;
    metaWithNative.name = "ohos.ability.withNativeModule";
    metaWithNative.value = "true";
    abilityRequest.abilityInfo.metadata.push_back(metaWithNative);

    AppExecFwk::Metadata metaPhase;
    metaPhase.name = "ohos.ability.startupPhase";
    metaPhase.value = "pre_foreground";
    abilityRequest.abilityInfo.metadata.push_back(metaPhase);

    AppExecFwk::Metadata metaSource;
    metaSource.name = "ohos.ability.nativeModuleSource";
    metaSource.value = "libtest.so";
    abilityRequest.abilityInfo.metadata.push_back(metaSource);

    AppExecFwk::Metadata metaFunc;
    metaFunc.name = "ohos.ability.nativeModuleFun";
    metaFunc.value = "OHMain";
    abilityRequest.abilityInfo.metadata.push_back(metaFunc);

    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);
}

/**
 * @tc.name: CreateAbilityRecord_0400
 * @tc.desc: withNativeModule=true, startupPhase=FOREGROUND, abilityNativeState should be NORMAL.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0400, TestSize.Level1)
{
    AbilityRequest abilityRequest;

    AppExecFwk::Metadata metaWithNative;
    metaWithNative.name = "ohos.ability.withNativeModule";
    metaWithNative.value = "true";
    abilityRequest.abilityInfo.metadata.push_back(metaWithNative);

    AppExecFwk::Metadata metaPhase;
    metaPhase.name = "ohos.ability.startupPhase";
    metaPhase.value = "foreground";
    abilityRequest.abilityInfo.metadata.push_back(metaPhase);

    AppExecFwk::Metadata metaSource;
    metaSource.name = "ohos.ability.nativeModuleSource";
    metaSource.value = "libtest.so";
    abilityRequest.abilityInfo.metadata.push_back(metaSource);

    AppExecFwk::Metadata metaFunc;
    metaFunc.name = "ohos.ability.nativeModuleFun";
    metaFunc.value = "OHMain";
    abilityRequest.abilityInfo.metadata.push_back(metaFunc);

    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NORMAL);
}

/**
 * @tc.name: CreateAbilityRecord_0500
 * @tc.desc: withNativeModule=false (explicit), abilityNativeState should remain NONE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0500, TestSize.Level1)
{
    AbilityRequest abilityRequest;

    AppExecFwk::Metadata metaWithNative;
    metaWithNative.name = "ohos.ability.withNativeModule";
    metaWithNative.value = "false";
    abilityRequest.abilityInfo.metadata.push_back(metaWithNative);

    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);
}

/**
 * @tc.name: CreateAbilityRecord_0600
 * @tc.desc: withNativeModule=true but nativeModuleSource missing, InitData disables native module.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0600, TestSize.Level1)
{
    AbilityRequest abilityRequest;

    AppExecFwk::Metadata metaWithNative;
    metaWithNative.name = "ohos.ability.withNativeModule";
    metaWithNative.value = "true";
    abilityRequest.abilityInfo.metadata.push_back(metaWithNative);

    // No nativeModuleSource metadata — InitData will set withNativeModule=false
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    // Source not specified, InitData disables native module
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);
}

/**
 * @tc.name: SetNativeState_0100
 * @tc.desc: Set and Get NativeState.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, SetNativeState_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    // Default is NONE
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);

    // Set to INIT
    abilityRecord->SetNativeState(AbilityNativeState::INIT);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);

    // Set to ATTACH
    abilityRecord->SetNativeState(AbilityNativeState::ATTACH);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::ATTACH);

    // Set to ON_CREATE
    abilityRecord->SetNativeState(AbilityNativeState::ON_CREATE);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::ON_CREATE);

    // Set to ON_FOREGROUND
    abilityRecord->SetNativeState(AbilityNativeState::ON_FOREGROUND);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::ON_FOREGROUND);

    // Set to NORMAL
    abilityRecord->SetNativeState(AbilityNativeState::NORMAL);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NORMAL);
}

/**
 * @tc.name: GetAbilityRecordType_0100
 * @tc.desc: Verify UIAbilityRecord returns UI_ABILITY type.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, GetAbilityRecordType_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetAbilityRecordType(), AbilityRecordType::UI_ABILITY);
}

/**
 * @tc.name: CheckStartPendingState_0100
 * @tc.desc: pendingState=INITIAL, should return true regardless of nativeState.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::INITIAL);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState());
}

/**
 * @tc.name: CheckStartPendingState_0200
 * @tc.desc: pendingState=FOREGROUND, nativeState=ON_CREATE, should return true.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0200, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->SetNativeState(AbilityNativeState::ON_CREATE);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState());
}

/**
 * @tc.name: CheckStartPendingState_0300
 * @tc.desc: pendingState=FOREGROUND, nativeState=NONE, should return false.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0300, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    // nativeState defaults to NONE
    EXPECT_FALSE(abilityRecord->CheckStartPendingState());
}

/**
 * @tc.name: CheckStartPendingState_0400
 * @tc.desc: pendingState=FOREGROUND, nativeState=ON_FOREGROUND, should return false.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0400, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->SetNativeState(AbilityNativeState::ON_FOREGROUND);
    EXPECT_FALSE(abilityRecord->CheckStartPendingState());
}

/**
 * @tc.name: CheckStartPendingState_0500
 * @tc.desc: pendingState=BACKGROUND, should return false regardless of nativeState.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0500, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetNativeState(AbilityNativeState::ON_CREATE);
    EXPECT_FALSE(abilityRecord->CheckStartPendingState());
}

/**
 * @tc.name: CheckStartPendingState_0600
 * @tc.desc: pendingState=INITIAL with various nativeState values, all should return true.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0600, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::INITIAL);
    abilityRecord->SetNativeState(AbilityNativeState::NORMAL);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState());

    abilityRecord->SetNativeState(AbilityNativeState::INIT);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState());

    abilityRecord->SetNativeState(AbilityNativeState::ATTACH);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState());
}
}  // namespace AAFwk
}  // namespace OHOS
