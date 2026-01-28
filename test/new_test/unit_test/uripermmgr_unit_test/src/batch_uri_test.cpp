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
#include "batch_uri.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class BatchUriTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<BatchUri> batchUri_;
};

void BatchUriTest::SetUpTestCase() {}

void BatchUriTest::TearDownTestCase() {}

void BatchUriTest::SetUp()
{
    batchUri_ = std::make_unique<BatchUri>();
}

void BatchUriTest::TearDown()
{
    batchUri_ = nullptr;
}

/**
 * @tc.name: BatchUri_Init_001
 * @tc.desc: Test BatchUri Init with valid file uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://com.example.app/data/storage/el2/base/test2.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0); // Should return positive count of valid uris
    EXPECT_EQ(batchUri_->totalUriCount, 2);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_001 end");
}

/**
 * @tc.name: BatchUri_Init_002
 * @tc.desc: Test BatchUri Init with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_002 start");

    std::vector<std::string> uriVec; // Empty vector
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(batchUri_->totalUriCount, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_002 end");
}

/**
 * @tc.name: BatchUri_Init_003
 * @tc.desc: Test BatchUri Init with media uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_003 start");

    std::vector<std::string> uriVec = {
        "file://media/image/test1.jpg",
        "file://media/image/test2.jpg"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_003 end");
}

/**
 * @tc.name: BatchUri_Init_004
 * @tc.desc: Test BatchUri Init with mixed uri types
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_004 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt",
        "file://media/image/test.jpg",
        "file://com.example.app/data/storage/el2/base/docs/test.doc"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);
    EXPECT_EQ(batchUri_->totalUriCount, 3);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_004 end");
}

/**
 * @tc.name: BatchUri_SetMediaUriCheckResult_001
 * @tc.desc: Test SetMediaUriCheckResult
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_SetMediaUriCheckResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SetMediaUriCheckResult_001 start");

    std::vector<bool> mediaUriResult = {true, false, true};

    batchUri_->SetMediaUriCheckResult(mediaUriResult);

    EXPECT_EQ(batchUri_->mediaUris.size(), mediaUriResult.size());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SetMediaUriCheckResult_001 end");
}

/**
 * @tc.name: BatchUri_SetMediaUriCheckResult_002
 * @tc.desc: Test SetMediaUriCheckResult with empty vector
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_SetMediaUriCheckResult_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SetMediaUriCheckResult_002 start");

    std::vector<bool> mediaUriResult; // Empty vector

    batchUri_->SetMediaUriCheckResult(mediaUriResult);

    EXPECT_TRUE(batchUri_->mediaUris.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SetMediaUriCheckResult_002 end");
}

/**
 * @tc.name: BatchUri_GetMediaUriToGrant_001
 * @tc.desc: Test GetMediaUriToGrant
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetMediaUriToGrant_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetMediaUriToGrant_001 start");

    std::vector<std::string> uriVec = {
        "file://media/image/test1.jpg",
        "file://media/image/test2.jpg"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    std::vector<std::string> uriToGrant;
    int32_t ret = batchUri_->GetMediaUriToGrant(uriToGrant);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetMediaUriToGrant_001 end");
}

/**
 * @tc.name: BatchUri_GetUriToGrantByMap_001
 * @tc.desc: Test GetUriToGrantByMap
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetUriToGrantByMap_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByMap_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    std::vector<std::string> uriToGrant;
    int32_t ret = batchUri_->GetUriToGrantByMap(uriToGrant);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByMap_001 end");
}

/**
 * @tc.name: BatchUri_GetUriToGrantByPolicy_001
 * @tc.desc: Test GetUriToGrantByPolicy
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetUriToGrantByPolicy_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_001 start");

    std::vector<PolicyInfo> docsPolicyInfoVec;
    std::vector<PolicyInfo> bundlePolicyInfoVec;

    PolicyInfo policy;
    policy.path = "/data/storage/el2/base/test.txt";
    policy.mode = 0;
    docsPolicyInfoVec.push_back(policy);

    int32_t ret = batchUri_->GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_001 end");
}

/**
 * @tc.name: BatchUri_GetUriToGrantByPolicy_002
 * @tc.desc: Test GetUriToGrantByPolicy with empty policy vectors
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetUriToGrantByPolicy_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_002 start");

    std::vector<PolicyInfo> docsPolicyInfoVec; // Empty
    std::vector<PolicyInfo> bundlePolicyInfoVec; // Empty

    int32_t ret = batchUri_->GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec);

    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_002 end");
}

/**
 * @tc.name: BatchUri_IsAllUriValid_001
 * @tc.desc: Test IsAllUriValid with all valid uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_IsAllUriValid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_IsAllUriValid_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://com.example.app/data/storage/el2/base/test2.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    bool result = batchUri_->IsAllUriValid();

    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_IsAllUriValid_001 end");
}

/**
 * @tc.name: BatchUri_IsAllUriValid_002
 * @tc.desc: Test IsAllUriValid with no valid uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_IsAllUriValid_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_IsAllUriValid_002 start");

    std::vector<std::string> uriVec; // No uris
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    bool result = batchUri_->IsAllUriValid();

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_IsAllUriValid_002 end");
}

/**
 * @tc.name: BatchUri_IsAllUriPermissioned_001
 * @tc.desc: Test IsAllUriPermissioned
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_IsAllUriPermissioned_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_IsAllUriPermissioned_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    bool result = batchUri_->IsAllUriPermissioned();

    // Result depends on permission checks
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_IsAllUriPermissioned_001 end");
}

/**
 * @tc.name: BatchUri_GetPermissionedUriCount_001
 * @tc.desc: Test GetPermissionedUriCount
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetPermissionedUriCount_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetPermissionedUriCount_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    int32_t count = batchUri_->GetPermissionedUriCount();

    EXPECT_GE(count, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetPermissionedUriCount_001 end");
}

/**
 * @tc.name: BatchUri_MixedUriTypes_001
 * @tc.desc: Test BatchUri with mixed media and file uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_MixedUriTypes_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_MixedUriTypes_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt",
        "file://media/image/test.jpg",
        "file://media/video/test.mp4",
        "file://com.example.app/data/storage/el2/base/docs/test.doc"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);
    EXPECT_EQ(batchUri_->totalUriCount, 4);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_MixedUriTypes_001 end");
}

/**
 * @tc.name: BatchUri_LargeUriList_001
 * @tc.desc: Test BatchUri with large uri list
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_LargeUriList_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_LargeUriList_001 start");

    std::vector<std::string> uriVec;
    for (int i = 0; i < 100; i++) {
        uriVec.push_back("file://com.example.app/data/storage/el2/base/test" + std::to_string(i) + ".txt");
    }

    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);
    EXPECT_EQ(batchUri_->totalUriCount, 100);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_LargeUriList_001 end");
}

/**
 * @tc.name: BatchUri_Init_005
 * @tc.desc: Test BatchUri Init with docs uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_005 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/docs/test1.doc",
        "file://com.example.app/data/storage/el2/base/docs/test2.doc"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_005 end");
}

/**
 * @tc.name: BatchUri_Init_006
 * @tc.desc: Test BatchUri Init with bundle uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_006 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/files/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_006 end");
}

/**
 * @tc.name: BatchUri_Init_007
 * @tc.desc: Test BatchUri Init with write permission
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_007 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_007 end");
}

/**
 * @tc.name: BatchUri_Init_008
 * @tc.desc: Test BatchUri Init with read write permission
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_Init_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_008 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_GT(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_Init_008 end");
}

/**
 * @tc.name: BatchUri_SetOtherUriCheckResult_001
 * @tc.desc: Test SetOtherUriCheckResult
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_SetOtherUriCheckResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SetOtherUriCheckResult_001 start");

    std::vector<bool> otherUriResult = {true, false, true, false};

    batchUri_->SetOtherUriCheckResult(otherUriResult);

    EXPECT_EQ(batchUri_->otherUris.size(), otherUriResult.size());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SetOtherUriCheckResult_001 end");
}

/**
 * @tc.name: BatchUri_GetUriToGrantByMap_002
 * @tc.desc: Test GetUriToGrantByMap with multiple uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetUriToGrantByMap_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByMap_002 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://com.example.app/data/storage/el2/base/test2.txt",
        "file://com.example.app/data/storage/el2/base/test3.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    std::vector<std::string> uriToGrant;
    int32_t ret = batchUri_->GetUriToGrantByMap(uriToGrant);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByMap_002 end");
}

/**
 * @tc.name: BatchUri_GetUriToGrantByPolicy_003
 * @tc.desc: Test GetUriToGrantByPolicy with multiple policies
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetUriToGrantByPolicy_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_003 start");

    std::vector<PolicyInfo> docsPolicyInfoVec;
    std::vector<PolicyInfo> bundlePolicyInfoVec;

    for (int i = 0; i < 3; i++) {
        PolicyInfo policy;
        policy.path = "/data/storage/el2/base/test" + std::to_string(i) + ".txt";
        policy.mode = 0;
        bundlePolicyInfoVec.push_back(policy);
    }

    int32_t ret = batchUri_->GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_003 end");
}

/**
 * @tc.name: BatchUri_GetUriToGrantByPolicy_004
 * @tc.desc: Test GetUriToGrantByPolicy bool version
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetUriToGrantByPolicy_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_004 start");

    std::vector<PolicyInfo> policyVec;
    for (int i = 0; i < 3; i++) {
        PolicyInfo policy;
        policy.path = "/data/storage/el2/base/test" + std::to_string(i) + ".txt";
        policy.mode = 0;
        policyVec.push_back(policy);
    }

    bool result = batchUri_->GetUriToGrantByPolicy(policyVec);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetUriToGrantByPolicy_004 end");
}

/**
 * @tc.name: BatchUri_GetPermissionedUriCount_002
 * @tc.desc: Test GetPermissionedUriCount with different counts
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetPermissionedUriCount_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetPermissionedUriCount_002 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://com.example.app/data/storage/el2/base/test2.txt",
        "file://com.example.app/data/storage/el2/base/test3.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    int32_t count = batchUri_->GetPermissionedUriCount();

    EXPECT_GE(count, 0);
    EXPECT_LE(count, 3);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetPermissionedUriCount_002 end");
}

/**
 * @tc.name: BatchUri_MediaUriProcessing_001
 * @tc.desc: Test media URI processing
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_MediaUriProcessing_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_MediaUriProcessing_001 start");

    std::vector<std::string> uriVec = {
        "file://media/image/test1.jpg",
        "file://media/image/test2.jpg",
        "file://media/video/test1.mp4"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    EXPECT_GT(batchUri_->mediaUris.size(), 0u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_MediaUriProcessing_001 end");
}

/**
 * @tc.name: BatchUri_DocsUriProcessing_001
 * @tc.desc: Test docs URI processing
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_DocsUriProcessing_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_DocsUriProcessing_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/docs/test1.doc",
        "file://com.example.app/data/storage/el2/base/docs/test2.doc"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    EXPECT_GT(batchUri_->otherUris.size(), 0u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_DocsUriProcessing_001 end");
}

/**
 * @tc.name: BatchUri_ContentUriProcessing_001
 * @tc.desc: Test content URI processing
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_ContentUriProcessing_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_ContentUriProcessing_001 start");

    std::vector<std::string> uriVec = {
        "content://com.example.app/data/test1.txt",
        "content://com.example.app/data/test2.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    EXPECT_GT(batchUri_->contentUris.size(), 0u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_ContentUriProcessing_001 end");
}

/**
 * @tc.name: BatchUri_InvalidUris_001
 * @tc.desc: Test BatchUri with invalid uris
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_InvalidUris_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_InvalidUris_001 start");

    std::vector<std::string> uriVec = {
        "invalid_uri_format",
        "http://example.com/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = batchUri_->Init(uriVec, mode);

    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_InvalidUris_001 end");
}

/**
 * @tc.name: BatchUri_InitWithBundleNames_001
 * @tc.desc: Test BatchUri Init with caller and target bundle names
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_InitWithBundleNames_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_InitWithBundleNames_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string callerBundleName = "com.example.caller";
    std::string targetBundleName = "com.example.target";

    int32_t ret = batchUri_->Init(uriVec, mode, callerBundleName, targetBundleName, false);

    EXPECT_GT(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_InitWithBundleNames_001 end");
}

/**
 * @tc.name: BatchUri_SandboxAccessPermission_001
 * @tc.desc: Test BatchUri Init with sandbox access permission
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_SandboxAccessPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SandboxAccessPermission_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string callerBundleName = "com.example.caller";
    std::string targetBundleName = "com.example.target";
    bool haveSandboxAccessPermission = true;

    int32_t ret = batchUri_->Init(uriVec, mode, callerBundleName, targetBundleName, haveSandboxAccessPermission);

    EXPECT_GT(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SandboxAccessPermission_001 end");
}

/**
 * @tc.name: BatchUri_GetNeedCheckProxyPermissionURI_001
 * @tc.desc: Test GetNeedCheckProxyPermissionURI
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_GetNeedCheckProxyPermissionURI_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetNeedCheckProxyPermissionURI_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    std::vector<PolicyInfo> proxyUrisByPolicy;
    batchUri_->GetNeedCheckProxyPermissionURI(proxyUrisByPolicy);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_GetNeedCheckProxyPermissionURI_001 end");
}

/**
 * @tc.name: BatchUri_SelectPermissionedUri_001
 * @tc.desc: Test SelectPermissionedUri
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, BatchUri_SelectPermissionedUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SelectPermissionedUri_001 start");

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://com.example.app/data/storage/el2/base/test2.txt"
    };
    uint32_t mode = Want::FLAG_AUTH_READ_URI_PERMISSION;

    batchUri_->Init(uriVec, mode);

    std::vector<Uri> uris;
    std::vector<int32_t> indexs;
    std::vector<std::string> uriStrVec;

    batchUri_->SelectPermissionedUri(uris, indexs, uriStrVec);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "BatchUri_SelectPermissionedUri_001 end");
}

} // namespace AAFwk
} // namespace OHOS
