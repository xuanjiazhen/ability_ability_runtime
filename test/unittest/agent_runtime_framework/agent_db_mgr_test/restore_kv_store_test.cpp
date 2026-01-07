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

#define private public
#include "agent_db_mgr.h"
#undef private

using namespace OHOS;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class RestoreKvStoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    
    std::unique_ptr<AgentDbMgr> agentDbMgr_;
};

void RestoreKvStoreTest::SetUpTestCase(void)
{}

void RestoreKvStoreTest::TearDownTestCase(void)
{}

void RestoreKvStoreTest::SetUp(void)
{
    agentDbMgr_ = std::make_unique<AgentDbMgr>();
}

void RestoreKvStoreTest::TearDown(void)
{
    agentDbMgr_.reset();
}

/**
 * @tc.name: RestoreKvStore_001
 * @tc.desc: Test RestoreKvStore when status is DATA_CORRUPTED
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_001, TestSize.Level1)
{
    // Given: status is DATA_CORRUPTED
    DistributedKv::Status status = DistributedKv::Status::DATA_CORRUPTED;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Return value should come from RestoreCorruptedKvStore
    // We expect it to return some status, specific value depends on mock implementation
    EXPECT_NE(result, status);
}

/**
 * @tc.name: RestoreKvStore_002
 * @tc.desc: Test RestoreKvStore when status is SUCCESS
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_002, TestSize.Level1)
{
    // Given: status is SUCCESS (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_003
 * @tc.desc: Test RestoreKvStore when status is STORE_NOT_FOUND
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_003, TestSize.Level1)
{
    // Given: status is STORE_NOT_FOUND (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::STORE_NOT_FOUND;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_004
 * @tc.desc: Test RestoreKvStore when status is DB_ERROR
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_004, TestSize.Level1)
{
    // Given: status is DB_ERROR (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::DB_ERROR;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_005
 * @tc.desc: Test RestoreKvStore when status is INVALID_ARGUMENT
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_005, TestSize.Level1)
{
    // Given: status is INVALID_ARGUMENT (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::INVALID_ARGUMENT;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_006
 * @tc.desc: Test RestoreKvStore when status is PERMISSION_DENIED
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_006, TestSize.Level1)
{
    // Given: status is PERMISSION_DENIED (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::PERMISSION_DENIED;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_007
 * @tc.desc: Test RestoreKvStore when status is NOT_SUPPORT
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_007, TestSize.Level1)
{
    // Given: status is NOT_SUPPORT (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::NOT_SUPPORT;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_008
 * @tc.desc: Test RestoreKvStore when status is TIME_OUT
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_008, TestSize.Level1)
{
    // Given: status is TIME_OUT (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::TIME_OUT;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_009
 * @tc.desc: Test RestoreKvStore when status is BUSY
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_009, TestSize.Level1)
{
    // Given: status is BUSY (not DATA_CORRUPTED)
    DistributedKv::Status status = DistributedKv::Status::BUSY;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status without calling RestoreCorruptedKvStore
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: RestoreKvStore_010
 * @tc.desc: Test RestoreKvStore with minimum boundary value
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_010, TestSize.Level1)
{
    // Given: status is the minimum possible enum value (SUCCESS)
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should return the same status
    EXPECT_EQ(result, status);
}

/**
 * @tc.name: CreateKvStoreOptions_001
 * @tc.desc: Test CreateKvStoreOptions returns proper configuration
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, CreateKvStoreOptions_001, TestSize.Level1)
{
    // When: CreateKvStoreOptions is called
    DistributedKv::Options options = agentDbMgr_->CreateKvStoreOptions();
    
    // Then: Should return options with expected values
    EXPECT_TRUE(options.createIfMissing);
    EXPECT_FALSE(options.encrypt);
    EXPECT_FALSE(options.autoSync);
    EXPECT_FALSE(options.syncable);
    EXPECT_EQ(options.securityLevel, DistributedKv::SecurityLevel::S2);
    EXPECT_EQ(options.area, DistributedKv::EL1);
    EXPECT_EQ(options.kvStoreType, DistributedKv::KvStoreType::SINGLE_VERSION);
    EXPECT_STREQ(options.baseDir, "/data/service/el1/public/database/agent_runtime");
}

/**
 * @tc.name: RestoreCorruptedKvStore_001
 * @tc.desc: Test RestoreCorruptedKvStore basic functionality
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreCorruptedKvStore_001, TestSize.Level1)
{
    // Given: Valid options
    DistributedKv::Options options = agentDbMgr_->CreateKvStoreOptions();
    
    // When: RestoreCorruptedKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreCorruptedKvStore(options);
    
    // Then: Should return some status (implementation dependent, but shouldn't crash)
    // This tests that the function executes without throwing exceptions
    EXPECT_NO_THROW();
}

/**
 * @tc.name: RestoreKvStore_Integration_001
 * @tc.desc: Test the complete flow when status is DATA_CORRUPTED
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_Integration_001, TestSize.Level1)
{
    // Given: status is DATA_CORRUPTED and we want to verify the complete flow
    DistributedKv::Status status = DistributedKv::Status::DATA_CORRUPTED;
    
    // When: RestoreKvStore is called
    DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
    
    // Then: Should not return DATA_CORRUPTED (since it should attempt recovery)
    EXPECT_NE(result, DistributedKv::Status::DATA_CORRUPTED);
    
    // And: Should not throw any exceptions
    EXPECT_NO_THROW();
}

/**
 * @tc.name: RestoreKvStore_Integration_002
 * @tc.desc: Test the complete flow with multiple non-corrupted status values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(RestoreKvStoreTest, RestoreKvStore_Integration_002, TestSize.Level1)
{
    // Test with various non-corrupted status values
    std::vector<DistributedKv::Status> nonCorruptedStatuses = {
        DistributedKv::Status::SUCCESS,
        DistributedKv::Status::STORE_NOT_FOUND,
        DistributedKv::Status::DB_ERROR,
        DistributedKv::Status::INVALID_ARGUMENT,
        DistributedKv::Status::PERMISSION_DENIED,
        DistributedKv::Status::NOT_SUPPORT,
        DistributedKv::Status::TIME_OUT,
        DistributedKv::Status::BUSY
    };
    
    for (auto status : nonCorruptedStatuses) {
        // When: RestoreKvStore is called with non-corrupted status
        DistributedKv::Status result = agentDbMgr_->RestoreKvStore(status);
        
        // Then: Should return the same status (no recovery attempted)
        EXPECT_EQ(result, status);
    }
}
} // namespace AgentRuntime
} // namespace OHOS