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
#define private public
#define protected public
#include "resource_config_helper.h"
#undef protected
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace AbilityRuntime;

class ResourceConfigHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    ResourceConfigHelper helper_;
};

void ResourceConfigHelperTest::SetUpTestCase(void)
{}

void ResourceConfigHelperTest::TearDownTestCase(void)
{}

void ResourceConfigHelperTest::SetUp(void)
{}

void ResourceConfigHelperTest::TearDown(void)
{}

/**
 * @tc.number: SetThemeSkin_0100
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets the theme skin value with a normal numeric string.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0100 start";
    std::string themeSkin = "100";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0100 end";
}

/**
 * @tc.number: SetThemeSkin_0200
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets an empty theme skin value.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0200 start";
    std::string themeSkin = "";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0200 end";
}

/**
 * @tc.number: SetThemeSkin_0300
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a zero theme skin value.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0300 start";
    std::string themeSkin = "0";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0300 end";
}

/**
 * @tc.number: SetThemeSkin_0400
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a large theme skin value.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0400 start";
    std::string themeSkin = "4294967295";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0400 end";
}

/**
 * @tc.number: SetThemeSkin_0500
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a hexadecimal theme skin value.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0500, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0500 start";
    std::string themeSkin = "0xFF";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0500 end";
}

/**
 * @tc.number: SetThemeSkin_0600
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets an alphanumeric theme skin value.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0600, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0600 start";
    std::string themeSkin = "abc123";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0600 end";
}

/**
 * @tc.number: SetThemeSkin_0700
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a theme skin value with special characters.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0700, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0700 start";
    std::string themeSkin = "skin_v1.0";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0700 end";
}

/**
 * @tc.number: SetThemeSkin_0800
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a theme skin value with unicode characters.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0800, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0800 start";
    std::string themeSkin = "皮肤";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0800 end";
}

/**
 * @tc.number: SetThemeSkin_0900
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a theme skin value with leading zeros.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_0900, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_0900 start";
    std::string themeSkin = "00123";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_0900 end";
}

/**
 * @tc.number: SetThemeSkin_1000
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a negative theme skin value.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_1000, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_1000 start";
    std::string themeSkin = "-1";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_1000 end";
}

/**
 * @tc.number: SetThemeSkin_1100
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that multiple calls to SetThemeSkin correctly update the theme skin value.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_1100, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_1100 start";
    std::string themeSkin1 = "100";
    std::string themeSkin2 = "200";
    std::string themeSkin3 = "300";

    helper_.SetThemeSkin(themeSkin1);
    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin1.c_str());

    helper_.SetThemeSkin(themeSkin2);
    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin2.c_str());

    helper_.SetThemeSkin(themeSkin3);
    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin3.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_1100 end";
}

/**
 * @tc.number: SetThemeSkin_1200
 * @tc.name: SetThemeSkin
 * @tc.desc: Verify that SetThemeSkin correctly sets a theme skin value with whitespace.
 */
HWTEST_F(ResourceConfigHelperTest, SetThemeSkin_1200, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "SetThemeSkin_1200 start";
    std::string themeSkin = "  100  ";
    helper_.SetThemeSkin(themeSkin);

    EXPECT_STREQ(helper_.themeSkin_.c_str(), themeSkin.c_str());
    GTEST_LOG_(INFO) << "SetThemeSkin_1200 end";
}

} // namespace AppExecFwk
} // namespace OHOS
