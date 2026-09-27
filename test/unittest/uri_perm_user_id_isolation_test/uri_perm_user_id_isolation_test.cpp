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
#include "mock_native_token.h"
#define private public
#define protected public
#include "uri_permission_manager_stub_impl.h"
#undef private
#undef protected
#include "uri.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {

class UriPermUserIdIsolationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UriPermUserIdIsolationTest::SetUpTestCase() {}
void UriPermUserIdIsolationTest::TearDownTestCase() {}
void UriPermUserIdIsolationTest::SetUp() {}
void UriPermUserIdIsolationTest::TearDown() {}

/**
 * @tc.name: UriPermUserIdIsolation_001
 * @tc.desc: VM-3 - Multi-foreground user concurrent Grant/Revoke, userId isolation no cross-talk (R-3/AC-2.1)
 * @tc.type: FUNC
 */
HWTEST_F(UriPermUserIdIsolationTest, UriPermUserIdIsolation_001, TestSize.Level1)
{
    auto stubImpl = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(stubImpl, nullptr);

    uint32_t userATokenId = 10001;
    uint32_t userBTokenId = 20001;
    int32_t userAId = 100;
    int32_t userBId = 101;
    std::string uriStr = "file://docs/test/path";
    uint32_t flag = 1;

    stubImpl->AddTempUriPermission(uriStr, flag, userATokenId, userATokenId, userAId);
    stubImpl->AddTempUriPermission(uriStr, flag, userBTokenId, userBTokenId, userBId);

    {
        std::lock_guard<std::mutex> guard(stubImpl->mutex_);
        auto search = stubImpl->uriMap_.find(uriStr);
        ASSERT_NE(search, stubImpl->uriMap_.end());
        EXPECT_EQ(search->second.size(), 2u);
        bool foundA = false;
        bool foundB = false;
        for (const auto& info : search->second) {
            if (info.targetTokenId == userATokenId) {
                EXPECT_EQ(info.userId, userAId);
                foundA = true;
            }
            if (info.targetTokenId == userBTokenId) {
                EXPECT_EQ(info.userId, userBId);
                foundB = true;
            }
        }
        EXPECT_TRUE(foundA);
        EXPECT_TRUE(foundB);
    }

    stubImpl->RevokeMapUriPermission(userATokenId);

    {
        std::lock_guard<std::mutex> guard(stubImpl->mutex_);
        auto search = stubImpl->uriMap_.find(uriStr);
        if (search != stubImpl->uriMap_.end()) {
            bool foundB = false;
            for (const auto& info : search->second) {
                EXPECT_NE(info.targetTokenId, userATokenId);
                if (info.targetTokenId == userBTokenId) {
                    foundB = true;
                }
            }
            EXPECT_TRUE(foundB);
        }
    }
}

/**
 * @tc.name: UriPermUserIdIsolation_002
 * @tc.desc: VM-4 - Grant to user A, Revoke from user B, user A permission preserved (R-4/AC-2.2)
 * @tc.type: FUNC
 */
HWTEST_F(UriPermUserIdIsolationTest, UriPermUserIdIsolation_002, TestSize.Level1)
{
    auto stubImpl = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(stubImpl, nullptr);

    uint32_t userATokenId = 10001;
    uint32_t userBTokenId = 20001;
    int32_t userAId = 100;
    int32_t userBId = 101;
    std::string uriStr = "file://docs/test/crossuser";
    uint32_t flag = 1;

    stubImpl->AddTempUriPermission(uriStr, flag, userATokenId, userATokenId, userAId);

    stubImpl->RevokeMapUriPermission(userBTokenId);

    {
        std::lock_guard<std::mutex> guard(stubImpl->mutex_);
        auto search = stubImpl->uriMap_.find(uriStr);
        ASSERT_NE(search, stubImpl->uriMap_.end());
        bool foundA = false;
        for (const auto& info : search->second) {
            if (info.targetTokenId == userATokenId) {
                foundA = true;
                EXPECT_EQ(info.userId, userAId);
            }
        }
        EXPECT_TRUE(foundA);
    }
}

/**
 * @tc.name: UriPermUserIdIsolation_003
 * @tc.desc: VM-5 - Single foreground user scenario, Revoke behavior consistent with pre-adaptation (R-5/AC-2.3)
 * @tc.type: FUNC
 */
HWTEST_F(UriPermUserIdIsolationTest, UriPermUserIdIsolation_003, TestSize.Level1)
{
    auto stubImpl = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(stubImpl, nullptr);

    uint32_t tokenId = 10001;
    int32_t userId = 100;
    std::string uriStr = "file://docs/test/single";
    uint32_t flag = 1;

    stubImpl->AddTempUriPermission(uriStr, flag, tokenId, tokenId, userId);

    stubImpl->RevokeMapUriPermission(tokenId);

    {
        std::lock_guard<std::mutex> guard(stubImpl->mutex_);
        auto search = stubImpl->uriMap_.find(uriStr);
        EXPECT_EQ(search, stubImpl->uriMap_.end());
    }
}

/**
 * @tc.name: UriPermUserIdIsolation_004
 * @tc.desc: VM-5b - Single foreground user scenario, userId=-1 default value backward compat
 * @tc.type: FUNC
 */
HWTEST_F(UriPermUserIdIsolationTest, UriPermUserIdIsolation_004, TestSize.Level1)
{
    auto stubImpl = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(stubImpl, nullptr);

    uint32_t tokenId = 10001;
    std::string uriStr = "file://docs/test/compat";
    uint32_t flag = 1;

    stubImpl->AddTempUriPermission(uriStr, flag, tokenId, tokenId);

    {
        std::lock_guard<std::mutex> guard(stubImpl->mutex_);
        auto search = stubImpl->uriMap_.find(uriStr);
        ASSERT_NE(search, stubImpl->uriMap_.end());
        ASSERT_FALSE(search->second.empty());
        EXPECT_EQ(search->second.front().userId, -1);
    }

    stubImpl->RevokeMapUriPermission(tokenId);

    {
        std::lock_guard<std::mutex> guard(stubImpl->mutex_);
        auto search = stubImpl->uriMap_.find(uriStr);
        EXPECT_EQ(search, stubImpl->uriMap_.end());
    }
}
}  // namespace AAFwk
}  // namespace OHOS
