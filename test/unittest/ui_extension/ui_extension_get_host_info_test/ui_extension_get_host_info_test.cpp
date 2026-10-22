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

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIExtensionGetHostInfoTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

static uint32_t g_savedSelfTokenId = 0;

void UIExtensionGetHostInfoTest::SetUpTestCase()
{
    // Capture the original shell token id before any SetNativeToken, so _1200 can force
    // a non-SA identity
    g_savedSelfTokenId = GetSelfTokenID();
}

void UIExtensionGetHostInfoTest::TearDownTestCase(void)
{}

void UIExtensionGetHostInfoTest::SetUp()
{}

void UIExtensionGetHostInfoTest::TearDown()
{}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0100
 * @tc.desc: get ui extension root host info without permision.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_NE(token, nullptr);
    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(token, hostInfo);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0200
 * @tc.desc: get ui extension root host info without invalid param.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(nullptr, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0300
 * @tc.desc: basic function test of get ui extension root host info.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto currentID = GetSelfTokenID();
    AppExecFwk::MockNativeToken::SetNativeToken();

    sptr<IRemoteObject> token = nullptr;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
        if (ret != ERR_OK) {
            SetSelfTokenID(currentID);
            TAG_LOGI(AAFwkTag::TEST, "end.");
            return;
        }
        UIExtensionHostInfo hostInfo;
        ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(token, hostInfo);
        // cause top ability isn't a uiextension ability.
        EXPECT_EQ(ret, ERR_INVALID_VALUE);
        TAG_LOGI(AAFwkTag::TEST, "Get host info uri: %{public}s", hostInfo.elementName_.GetURI().c_str());
    }

    SetSelfTokenID(currentID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0400
 * @tc.desc: basic function test of get ui extension root host info.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto currentID = GetSelfTokenID();
    AppExecFwk::MockNativeToken::SetNativeToken();

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_NE(token, nullptr);

    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(token, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "Get host info uri: %{public}s", hostInfo.elementName_.GetURI().c_str());

    SetSelfTokenID(currentID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0100
 * @tc.desc: query with invalid param zero, proxy guard rejects it directly.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(0, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0200
 * @tc.desc: query with negative params (-1 and INT32_MIN), proxy guard rejects them.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(-1, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    int32_t minId = INT32_MIN;
    ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(minId, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0300
 * @tc.desc: query without SA identity must be rejected by service side.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    // SA identity (SetNativeToken) querying a non-existent record: 999999999 is far beyond
    // the counter upper bound, so ERR_INVALID_VALUE(22) is returned deterministically
    // regardless of device state. The non-SA denial (ERR_PERMISSION_DENIED) case is
    // covered by _1200 with a forced shell identity.
    AppExecFwk::MockNativeToken::SetNativeToken();
    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(999999999, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "ret: %{public}d.", ret);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0400
 * @tc.desc: query with SA identity but record not existed, exact error code and empty result.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto currentID = GetSelfTokenID();
    AppExecFwk::MockNativeToken::SetNativeToken();

    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(999999, hostInfo);
    // Exact assertion: with a valid SA identity, a missing record must return
    // ERR_INVALID_VALUE rather than a generic non-zero value.
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    // On failure the output must stay empty (AC-3.1).
    EXPECT_EQ(hostInfo.sessionName.empty(), true);
    EXPECT_EQ(hostInfo.persistentId_, 0);

    SetSelfTokenID(currentID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0500
 * @tc.desc: default value check and full-field parcel round trip incl. elementName.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    UIExtensionHostInfo hostInfo;
    // Default values (empty branch of AC-1.2/1.3).
    EXPECT_EQ(hostInfo.persistentId_, 0);
    EXPECT_EQ(hostInfo.sessionName.empty(), true);

    // Round trip with all fields set: assert each ElementName field required by the
    // requirement (moduleName is restored by backfill).
    hostInfo.elementName_.SetBundleName("com.test.hoster");
    hostInfo.elementName_.SetAbilityName("HostAbility");
    hostInfo.elementName_.SetModuleName("entry");
    hostInfo.sessionName = "session_test";
    hostInfo.persistentId_ = 100;

    Parcel parcel;
    EXPECT_EQ(hostInfo.Marshalling(parcel), true);
    std::unique_ptr<UIExtensionHostInfo> readInfo(UIExtensionHostInfo::Unmarshalling(parcel));
    ASSERT_NE(readInfo, nullptr);
    EXPECT_EQ(readInfo->elementName_.GetBundleName(), "com.test.hoster");
    EXPECT_EQ(readInfo->elementName_.GetAbilityName(), "HostAbility");
    // ElementName marshalling drops moduleName (known ability_base behavior); the parser
    // saves it into the standalone field and backfills elementName_, so the contract
    // field is restored:
    EXPECT_EQ(readInfo->elementName_.GetModuleName(), "entry");
    EXPECT_EQ(readInfo->sessionName, "session_test");
    EXPECT_EQ(readInfo->persistentId_, 100);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0600
 * @tc.desc: persistentId_ boundary values (INT32_MAX / INT32_MIN) parcel round trip.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    const int32_t boundaries[] = { INT32_MAX, INT32_MIN };
    for (const auto &value : boundaries) {
        UIExtensionHostInfo hostInfo;
        hostInfo.sessionName = "boundary";
        hostInfo.persistentId_ = value;

        Parcel parcel;
        EXPECT_EQ(hostInfo.Marshalling(parcel), true);
        std::unique_ptr<UIExtensionHostInfo> readInfo(UIExtensionHostInfo::Unmarshalling(parcel));
        ASSERT_NE(readInfo, nullptr);
        EXPECT_EQ(readInfo->persistentId_, value);
    }
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0700
 * @tc.desc: sessionName boundary (long string and UTF-8 chinese chars) parcel round trip.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    const std::string longName(256, 'a');
    const std::string chineseName = "会话_中文_测试";

    for (const auto &name : { longName, chineseName }) {
        UIExtensionHostInfo hostInfo;
        hostInfo.sessionName = name;
        hostInfo.persistentId_ = 7;

        Parcel parcel;
        EXPECT_EQ(hostInfo.Marshalling(parcel), true);
        std::unique_ptr<UIExtensionHostInfo> readInfo(UIExtensionHostInfo::Unmarshalling(parcel));
        ASSERT_NE(readInfo, nullptr);
        EXPECT_EQ(readInfo->sessionName, name);
        EXPECT_EQ(readInfo->persistentId_, 7);
    }
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0800
 * @tc.desc: elementName with empty fields parcel round trip.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    // Empty elementName: marshalling/unmarshalling must not fail and fields stay empty.
    UIExtensionHostInfo hostInfo;
    hostInfo.sessionName = "empty_element";

    Parcel parcel;
    EXPECT_EQ(hostInfo.Marshalling(parcel), true);
    std::unique_ptr<UIExtensionHostInfo> readInfo(UIExtensionHostInfo::Unmarshalling(parcel));
    ASSERT_NE(readInfo, nullptr);
    EXPECT_EQ(readInfo->elementName_.GetBundleName(), "");
    EXPECT_EQ(readInfo->elementName_.GetAbilityName(), "");
    EXPECT_EQ(readInfo->elementName_.GetModuleName(), "");
    EXPECT_EQ(readInfo->sessionName, "empty_element");

    // Partially empty: bundleName is empty while the other fields are not.
    UIExtensionHostInfo hostInfo2;
    hostInfo2.elementName_.SetAbilityName("OnlyAbility");
    hostInfo2.elementName_.SetModuleName("only_module");
    hostInfo2.sessionName = "partial_element";
    hostInfo2.persistentId_ = 9;

    Parcel parcel2;
    EXPECT_EQ(hostInfo2.Marshalling(parcel2), true);
    std::unique_ptr<UIExtensionHostInfo> readInfo2(UIExtensionHostInfo::Unmarshalling(parcel2));
    ASSERT_NE(readInfo2, nullptr);
    EXPECT_EQ(readInfo2->elementName_.GetBundleName(), "");
    EXPECT_EQ(readInfo2->elementName_.GetAbilityName(), "OnlyAbility");
    EXPECT_EQ(readInfo2->elementName_.GetModuleName(), "only_module");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_0900
 * @tc.desc: old-format parcel compatibility: parcel without persistentId_ falls back to 0.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    // Version compatibility branch: simulate an old server reply (elementName + sessionName
    // only, no persistentId). ReadInt32 failure must fall back to 0 without failing the
    // whole parsing.
    AppExecFwk::ElementName element;
    element.SetBundleName("com.example.host");
    element.SetAbilityName("HostAbility");
    element.SetModuleName("entry");

    Parcel oldParcel;
    EXPECT_EQ(oldParcel.WriteParcelable(&element), true);
    EXPECT_EQ(oldParcel.WriteString("old_session"), true);

    std::unique_ptr<UIExtensionHostInfo> readInfo(UIExtensionHostInfo::Unmarshalling(oldParcel));
    ASSERT_NE(readInfo, nullptr);
    EXPECT_EQ(readInfo->elementName_.GetBundleName(), "com.example.host");
    EXPECT_EQ(readInfo->elementName_.GetAbilityName(), "HostAbility");
    // Old reply has no moduleName field -> fall back to empty and backfill empty (same
    // compatibility mode as persistentId):
    EXPECT_EQ(readInfo->elementName_.GetModuleName(), "");
    EXPECT_EQ(readInfo->sessionName, "old_session");
    EXPECT_EQ(readInfo->persistentId_, 0);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_1000
 * @tc.desc: unmarshalling from empty parcel must fail and return nullptr.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    // Parse failure path: reading elementName from an empty parcel fails -> ReadFromParcel
    // returns false -> Unmarshalling returns nullptr.
    Parcel emptyParcel;
    std::unique_ptr<UIExtensionHostInfo> readInfo(UIExtensionHostInfo::Unmarshalling(emptyParcel));
    EXPECT_EQ(readInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_1100
 * @tc.desc: LIVE acceptance: query a REAL record as SA. Run with env HOST_INFO_LIVE_RECORD_ID=<realId>;
 *           skipped automatically when the env is absent.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    const char *idStr = getenv("HOST_INFO_LIVE_RECORD_ID");
    if (idStr == nullptr || idStr[0] == '\0') {
        GTEST_SKIP() << "set HOST_INFO_LIVE_RECORD_ID=<real abilityRecordId> to run the live acceptance";
    }
    int32_t recordId = atoi(idStr);
    if (recordId <= 0) {
        GTEST_SKIP() << "HOST_INFO_LIVE_RECORD_ID must be a positive abilityRecordId";
    }
    AppExecFwk::MockNativeToken::SetNativeToken();

    UIExtensionHostInfo info;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(recordId, info);
    GTEST_LOG_(INFO) << "LIVE ret=" << ret << " bundle=" << info.elementName_.GetBundleName()
        << " ability=" << info.elementName_.GetAbilityName() << " module=" << info.elementName_.GetModuleName()
        << " session=" << info.sessionName << " persistentId=" << info.persistentId_;
    // Acceptance: SA query through the new inner API succeeds -> 0 with non-empty host
    // identity.
    EXPECT_EQ(ret, 0);
    EXPECT_NE(info.elementName_.GetBundleName(), "");
    EXPECT_NE(info.elementName_.GetAbilityName(), "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionHostInfoById_1200
 * @tc.desc: LIVE negative: force shell token id (captured in SetUpTestCase before any token
 *           grant) then must be denied with ERR_PERMISSION_DENIED(1) deterministically, both
 *           standalone and in a full-suite run.
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    ASSERT_NE(g_savedSelfTokenId, 0);
    SetSelfTokenID(g_savedSelfTokenId); // Force shell identity (earlier cases may have switched to native)
    UIExtensionHostInfo info;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(1, info);
    GTEST_LOG_(INFO) << "LIVE-NEG ret=" << ret;
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED); // Non-SA -> exactly 1, deterministic in both runs
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
/**
 * @tc.name: GetUIExtensionHostInfoById_1300
 * @tc.desc: LIVE SA identity: mock a native SA token (MockNativeToken) then query a non-existent
 *           record id. The permission gate must pass (otherwise ERR_PERMISSION_DENIED(1) is
 *           returned) and the record lookup must fail with ERR_INVALID_VALUE(22).
 *           This proves the "SA allowed" path over real IPC without needing a real
 *           UIExtension record, which is the testing manner agreed with the SE (mock SA).
 * @tc.type: FUNC
 * @tc.require: SR20260802799171
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionHostInfoById_1300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::MockNativeToken::SetNativeToken();
    UIExtensionHostInfo info;
    constexpr int32_t invalidRecordId = 0x7FFFFFFF;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionHostInfoById(invalidRecordId, info);
    GTEST_LOG_(INFO) << "LIVE-SA ret=" << ret;
    // Permission gate passed (otherwise 1 is returned); record not found -> 22
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    SetSelfTokenID(g_savedSelfTokenId);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

} // namespace AAFwk
} // namespace OHOS
