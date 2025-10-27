/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "foreground_app_connection_manager.h"
#undef private
#include "foreground_app_connection_errors.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {
class ForegroundAppConnectionManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ForegroundAppConnectionManagerTest::SetUpTestCase(void)
{}
void ForegroundAppConnectionManagerTest::TearDownTestCase(void)
{}
void ForegroundAppConnectionManagerTest::SetUp(void)
{}
void ForegroundAppConnectionManagerTest::TearDown(void)
{}

/*
 * Feature: ForegroundAppConnectionManager
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterObserver
 */
HWTEST_F(ForegroundAppConnectionManagerTest, RegisterObserver_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    sptr<AbilityRuntime::IForegroundAppConnection> observer = nullptr;
    auto res = foregroundAppConnectionManager->RegisterObserver(observer);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: ForegroundAppConnectionManager
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterObserver
 */
HWTEST_F(ForegroundAppConnectionManagerTest, UnregisterObserver_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    sptr<AbilityRuntime::IForegroundAppConnection> observer = nullptr;
    foregroundAppConnectionManager->UnregisterObserver(observer);
}

/*
 * Feature: ForegroundAppConnectionManager
 * Function: OnConnected
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager OnConnected
 * EnvConditions: NA
 * CaseDescription: Verify OnConnected
 */
HWTEST_F(ForegroundAppConnectionManagerTest, OnConnected_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    AbilityRuntime::ForegroundAppConnectionData data;
    foregroundAppConnectionManager->OnConnected(data);
}

/*
 * Feature: ForegroundAppConnectionManager
 * Function: OnDisconnected
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager OnDisconnected
 * EnvConditions: NA
 * CaseDescription: Verify OnDisconnected
 */
HWTEST_F(ForegroundAppConnectionManagerTest, OnDisconnected_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    AbilityRuntime::ForegroundAppConnectionData data;
    foregroundAppConnectionManager->OnDisconnected(data);
}

/*
 * Feature: ForegroundAppConnectionManager
 * Function: OnCallerStarted
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager OnCallerStarted
 * EnvConditions: NA
 * CaseDescription: Verify OnCallerStarted
 */
HWTEST_F(ForegroundAppConnectionManagerTest, OnCallerStarted_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    int32_t callerPid = 0;
    int32_t callerUid = 0;
    std::string bundleName = "bundleName";

    foregroundAppConnectionManager->OnCallerStarted(callerPid, callerUid, bundleName);
}

/*
 * Feature: ForegroundAppConnectionManager
 * Function: AbilityAddPidConnection
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager AbilityAddPidConnection
 * EnvConditions: NA
 * CaseDescription: Verify AbilityAddPidConnection
 */
HWTEST_F(ForegroundAppConnectionManagerTest, AbilityAddPidConnection_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    AbilityRuntime::ForegroundAppConnectionInfo info;
    int32_t abilityRecordId = 0;
    foregroundAppConnectionManager->AbilityAddPidConnection(info, abilityRecordId);
}
////

/*
 * Feature: ForegroundAppConnectionManager
 * Function: AbilityRemovePidConnection
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager AbilityRemovePidConnection
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRemovePidConnection
 */
HWTEST_F(ForegroundAppConnectionManagerTest, AbilityRemovePidConnection_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    int32_t callerPid = 0;
    int32_t targetPid = 0;
    int32_t abilityRecordId = 0;
    auto res = foregroundAppConnectionManager->AbilityRemovePidConnection(callerPid, targetPid,
        abilityRecordId);
}
///////


/*
 * Feature: ForegroundAppConnectionManager
 * Function: ProcessRemovePidConnection
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager ProcessRemovePidConnection
 * EnvConditions: NA
 * CaseDescription: Verify ProcessRemovePidConnection
 */
HWTEST_F(ForegroundAppConnectionManagerTest, ProcessRemovePidConnection_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    int32_t diedPid = 0;
    foregroundAppConnectionManager->ProcessRemovePidConnection(diedPid);
}

/*
 * Feature: ObserverDeathRecipient
 * Function: OnRemoteDied
 * SubFunction: NA
 * FunctionPoints: ForegroundAppConnectionManager OnRemoteDied
 * EnvConditions: NA
 * CaseDescription: Verify OnRemoteDied
 */
HWTEST_F(ForegroundAppConnectionManagerTest, OnRemoteDied_001, TestSize.Level1)
{
    auto foregroundAppConnectionManager = std::make_shared<ForegroundAppConnectionManager>();
    ASSERT_NE(foregroundAppConnectionManager, nullptr);
    ForegroundAppConnectionManager::ConnectionDeathRecipient::ConnectionDeathHandler handler;
    auto connectionDeathRecipient = std::make_shared<ForegroundAppConnectionManager::ConnectionDeathRecipient>(handler);
    wptr<IRemoteObject> remote;
    connectionDeathRecipient->OnRemoteDied(remote);
}
}  // namespace AAFwk
}  // namespace OHOS
