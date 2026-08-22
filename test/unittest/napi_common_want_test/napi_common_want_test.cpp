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
#include <gmock/gmock.h>

#include "access_token.h"
#include "napi_common_want.h"
#include "napi/native_api.h"
#include "token_setproc.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {

class NapiCommonWantTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NapiCommonWantTest::SetUpTestCase()
{
}

void NapiCommonWantTest::TearDownTestCase()
{
}

void NapiCommonWantTest::SetUp()
{
}

void NapiCommonWantTest::TearDown()
{
}

/**
 * @tc.name: BlackListFilter_NotFilter_0100
 * @tc.desc: proNameNotFilter matches the property name, should not be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_NotFilter_0100, Function | MediumTest | Level1)
{
    EXPECT_FALSE(BlackListFilter("action", "action"));
}

/**
 * @tc.name: BlackListFilter_WindowMode_0100
 * @tc.desc: PARAM_RESV_WINDOW_MODE is reserved and should be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_WindowMode_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(BlackListFilter(Want::PARAM_RESV_WINDOW_MODE, ""));
}

/**
 * @tc.name: BlackListFilter_DisplayId_0100
 * @tc.desc: PARAM_RESV_DISPLAY_ID is reserved and should be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_DisplayId_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(BlackListFilter(Want::PARAM_RESV_DISPLAY_ID, ""));
}

/**
 * @tc.name: BlackListFilter_NonReserved_0100
 * @tc.desc: A normal property name should not be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_NonReserved_0100, Function | MediumTest | Level1)
{
    EXPECT_FALSE(BlackListFilter("normalKey", ""));
}

/**
 * @tc.name: BlackListFilter_OriginStringNotFiltered_0100
 * @tc.desc: PARAM_SET_URI_WITH_ORIGIN_STRING must NOT be blacklisted so that the
 *          NAPI unwrap path can carry the pass-through flag to the server side.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_OriginStringNotFiltered_0100, Function | MediumTest | Level1)
{
    EXPECT_FALSE(BlackListFilter(Want::PARAM_SET_URI_WITH_ORIGIN_STRING, ""));
}

/**
 * @tc.name: SetUriWithPassThroughFlag_NoFlag_0100
 * @tc.desc: No pass-through flag set, uri is set directly.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_NoFlag_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    SetUriWithPassThroughFlag("https://www.example.com/path", want);
    EXPECT_EQ(want.GetUriString(), "https://www.example.com/path");
}

/**
 * @tc.name: SetUriWithPassThroughFlag_NonSystemApp_0100
 * @tc.desc: Pass-through flag set but caller is not a system app, uri is set directly.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_NonSystemApp_0100, Function | MediumTest | Level1)
{
    uint64_t originalToken = GetSelfTokenID();
    SetSelfTokenID(0);

    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING, true);
    SetUriWithPassThroughFlag("https://www.example.com/path", want);
    EXPECT_EQ(want.GetUriString(), "https://www.example.com/path");

    SetSelfTokenID(originalToken);
}

/**
 * @tc.name: SetUriWithPassThroughFlag_SystemApp_0100
 * @tc.desc: Pass-through flag set and caller is a system app, raw uri restored.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_SystemApp_0100, Function | MediumTest | Level1)
{
    uint64_t originalToken = GetSelfTokenID();

    uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_HAP;
    uint64_t fullTokenId = systemAppMask | tokenID;
    SetSelfTokenID(fullTokenId);

    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING, true);
    SetUriWithPassThroughFlag("https://www.example.com/path", want);
    EXPECT_EQ(want.GetUriString(), "https://www.example.com/path");

    SetSelfTokenID(originalToken);
}

/**
 * @tc.name: SetUriWithPassThroughFlag_FlagNotSetOnWant_0100
 * @tc.desc: Flag param absent on the want, direct SetUri regardless of caller identity.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_FlagNotSetOnWant_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    SetUriWithPassThroughFlag("https://www.example.com/path", want);
    EXPECT_EQ(want.GetUriString(), "https://www.example.com/path");
}

/**
 * @tc.name: UnwrapWant_NullParam_0100
 * @tc.desc: Null param is rejected by the type guard.
 */
HWTEST_F(NapiCommonWantTest, UnwrapWant_NullParam_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    EXPECT_FALSE(UnwrapWant(nullptr, nullptr, want));
}

/**
 * @tc.name: UnwrapWantParams_NullParam_0100
 * @tc.desc: Null param is rejected by the type guard.
 */
HWTEST_F(NapiCommonWantTest, UnwrapWantParams_NullParam_0100, Function | MediumTest | Level1)
{
    AAFwk::WantParams wantParams;
    EXPECT_FALSE(UnwrapWantParams(nullptr, nullptr, wantParams));
}
} // namespace AppExecFwk
} // namespace OHOS
