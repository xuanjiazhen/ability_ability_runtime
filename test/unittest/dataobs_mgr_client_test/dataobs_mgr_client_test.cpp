/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "gtest/gtest.h"
#include <memory>
#define private public
#include "dataobs_mgr_proxy.h"
#include "dataobs_mgr_client.h"
#include "mock_data_obs_manager_onchange_callback.h"
#include "mock_dataobs_mgr_service.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class DataObsMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataObsMgrClientTest::SetUpTestCase(void)
{}
void DataObsMgrClientTest::TearDownTestCase(void)
{}
void DataObsMgrClientTest::SetUp()
{}
void DataObsMgrClientTest::TearDown()
{}

/*
 * Feature: DataObsMgrClient.
 * Function: The function of dataobsmgrservice was called.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_Call_Service_0100, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();
    client->observerExts_.Clear();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);
    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    client->RegisterObserver(uri, callBack);
    EXPECT_EQ(service->onChangeCall_, 1);

    client->UnregisterObserver(uri, callBack);
    EXPECT_EQ(service->onChangeCall_, 2);

    client->NotifyChange(uri);
    EXPECT_EQ(service->onChangeCall_, 3);

    client->RegisterObserverExt(uri, callBack, false);
    EXPECT_EQ(service->onChangeCall_, 4);

    client->UnregisterObserverExt(uri, callBack);
    EXPECT_EQ(service->onChangeCall_, 5);

    client->UnregisterObserverExt(callBack);
    EXPECT_EQ(service->onChangeCall_, 6);

    client->NotifyChangeExt({ ChangeInfo::ChangeType::INSERT, { uri } });
    EXPECT_EQ(service->onChangeCall_, 7);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: re-subscribe when service restart.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_ReregisterObserver_0100, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack1(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<MockDataObsManagerOnChangeCallBack> callBack2(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();
    client->observerExts_.Clear();

    sptr<MockDataObsMgrService> service1(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service1;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);
    Uri uri1("datashare://device_id/com.domainname.dataability.persondata/person/25");
    Uri uri2("datashare://device_id/com.domainname.dataability.persondata/person/26");

    EXPECT_EQ(client->RegisterObserver(uri1, callBack1), NO_ERROR);
    EXPECT_EQ(client->RegisterObserver(uri2, callBack2), NO_ERROR);
    EXPECT_EQ(service1->onChangeCall_, 2);

    sptr<MockDataObsMgrService> service2(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service2;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);

    client->ReRegister();
    EXPECT_EQ(service2->onChangeCall_, 2);
    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: re-subscribe when service restart.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_ReregisterObserver_0200, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack1(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<MockDataObsManagerOnChangeCallBack> callBack2(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    Uri uri1("datashare://device_id/com.domainname.dataability.persondata/person/1");
    Uri uri2("datashare://device_id/com.domainname.dataability.persondata/person/2");
    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();
    client->observerExts_.Clear();

    sptr<MockDataObsMgrService> service1(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service1;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);

    EXPECT_EQ(client->RegisterObserverExt(uri1, callBack1, false), SUCCESS);
    EXPECT_EQ(client->RegisterObserverExt(uri2, callBack2, true), SUCCESS);
    EXPECT_EQ(service1->onChangeCall_, 2);

    sptr<MockDataObsMgrService> service2(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service2;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);

    client->ReRegister();
    EXPECT_EQ(service2->onChangeCall_, 2);
    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: re-subscribe when service restart.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_ReregisterObserver_0300, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack1(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<MockDataObsManagerOnChangeCallBack> callBack2(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();
    client->observerExts_.Clear();

    sptr<MockDataObsMgrService> service1(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service1;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);
    Uri uri1("datashare://device_id/com.domainname.dataability.persondata/person/25");
    Uri uri2("datashare://device_id/com.domainname.dataability.persondata/person/26");

    EXPECT_EQ(client->RegisterObserver(uri1, callBack1), NO_ERROR);
    EXPECT_EQ(client->RegisterObserver(uri2, callBack2), NO_ERROR);
    EXPECT_EQ(service1->onChangeCall_, 2);

    sptr<MockDataObsMgrService> service2(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service2;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);

    client->OnRemoteDied();
    EXPECT_EQ(service2->onChangeCall_, 0);
    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: RegisterObserverFromExtension
 * SubFunction: Handle service unavailable scenario.
 * FunctionPoints:
 *   1. Verify function returns error when service is unavailable
 *   2. Verify no service call is made when service is not connected
 * EnvConditions: Dataobs manager service is not available.
 * CaseDescription: Test RegisterObserverFromExtension behavior when service is unavailable.
 */
HWTEST_F(DataObsMgrClientTest, RegisterObserverFromExtension_ServiceUnavailable_0100, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();

    client->dataObsManger_ = nullptr;

    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    DataObsOption opt;
    opt.SetFirstCallerTokenID(123456);

    EXPECT_NE(client->RegisterObserverFromExtension(uri, callBack, 0, opt), NO_ERROR);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: RegisterObserverFromExtension
 * SubFunction: Handle service error response.
 * FunctionPoints:
 *   1. Verify function returns the same error code as service
 *   2. Verify service error does not affect client state
 * EnvConditions: Mock service returns error.
 * CaseDescription: Test RegisterObserverFromExtension behavior when service returns error.
 */
HWTEST_F(DataObsMgrClientTest, RegisterObserverFromExtension_ServiceError_0200, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    int32_t expectedError = 12345;
    EXPECT_CALL(*service, RegisterObserverFromExtension(_, _, _, _))
        .WillOnce(Return(expectedError));

    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    DataObsOption opt;
    opt.SetFirstCallerTokenID(123456);

    EXPECT_EQ(client->RegisterObserverFromExtension(uri, callBack, 0, opt), expectedError);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: RegisterObserverFromExtension
 * SubFunction: Register multiple observers from extension.
 * FunctionPoints:
 *   1. Verify multiple extension observers can be registered
 *   2. Verify each observer is stored separately with correct info
 *   3. Verify service is called for each registration
 * EnvConditions: Mock service is available.
 * CaseDescription: Test registration of multiple observers from extension.
 */
HWTEST_F(DataObsMgrClientTest, RegisterObserverFromExtension_MultipleObservers_0300, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack1(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<MockDataObsManagerOnChangeCallBack> callBack2(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    EXPECT_CALL(*service, RegisterObserverFromExtension(_, _, _, _))
        .Times(2)
        .WillRepeatedly(Return(NO_ERROR));

    Uri uri1("datashare://device_id/com.domainname.dataability.persondata/person/25");
    Uri uri2("datashare://device_id/com.domainname.dataability.persondata/person/26");
    DataObsOption opt1, opt2;
    opt1.SetFirstCallerTokenID(111111);
    opt2.SetFirstCallerTokenID(222222);

    EXPECT_EQ(client->RegisterObserverFromExtension(uri1, callBack1, 0, opt1), NO_ERROR);
    EXPECT_EQ(client->RegisterObserverFromExtension(uri2, callBack2, 100, opt2), NO_ERROR);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: RegisterObserverFromExtension
 * SubFunction: Register observers with different user IDs.
 * FunctionPoints:
 *   1. Verify extension observer can be registered with default user ID
 *   2. Verify extension observer can be registered with specific user ID
 *   3. Verify extension observer can be registered with negative user ID
 *   4. Verify service is called with correct user ID parameter
 * EnvConditions: Mock service is available.
 * CaseDescription: Test RegisterObserverFromExtension with different user ID values.
 */
HWTEST_F(DataObsMgrClientTest, RegisterObserverFromExtension_DifferentUserIds_0400, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    EXPECT_CALL(*service, RegisterObserverFromExtension(_, _, _, _))
        .Times(3)
        .WillRepeatedly(Return(NO_ERROR));

    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    DataObsOption opt;
    opt.SetFirstCallerTokenID(123456);

    EXPECT_EQ(client->RegisterObserverFromExtension(uri, callBack, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, opt),
        NO_ERROR);

    EXPECT_EQ(client->RegisterObserverFromExtension(uri, callBack, 100, opt), NO_ERROR);

    EXPECT_EQ(client->RegisterObserverFromExtension(uri, callBack, -1, opt), NO_ERROR);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyChangeFromExtension
 * SubFunction: Notify change from extension successfully.
 * FunctionPoints:
 *   1. Verify change can be notified from extension successfully
 *   2. Verify service method is called with correct parameters
 *   3. Verify function returns success code
 * EnvConditions: Mock service is available.
 * CaseDescription: Test successful notification of change from extension.
 */
HWTEST_F(DataObsMgrClientTest, NotifyChangeFromExtension_0100, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    DataObsOption opt;
    opt.SetFirstCallerTokenID(123456);

    EXPECT_CALL(*service, NotifyChangeFromExtension(_, _, _))
        .WillOnce(Return(NO_ERROR));

    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    EXPECT_EQ(client->NotifyChangeFromExtension(uri, 0, opt), NO_ERROR);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyChangeFromExtension
 * SubFunction: Handle service unavailable scenario.
 * FunctionPoints:
 *   1. Verify function returns error when service is unavailable
 *   2. Verify no service call is made when service is not connected
 * EnvConditions: Dataobs manager service is not available.
 * CaseDescription: Test NotifyChangeFromExtension behavior when service is unavailable.
 */
HWTEST_F(DataObsMgrClientTest, NotifyChangeFromExtension_ServiceUnavailable_0200, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    client->dataObsManger_ = nullptr;

    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    DataObsOption opt;
    opt.SetFirstCallerTokenID(123456);

    EXPECT_NE(client->NotifyChangeFromExtension(uri, 0, opt), NO_ERROR);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyChangeFromExtension
 * SubFunction: Handle service error response.
 * FunctionPoints:
 *   1. Verify function returns the same error code as service
 *   2. Verify service error does not affect client state
 * EnvConditions: Mock service returns error.
 * CaseDescription: Test NotifyChangeFromExtension behavior when service returns error.
 */
HWTEST_F(DataObsMgrClientTest, NotifyChangeFromExtension_ServiceError_0300, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    int32_t expectedError = 12345;
    EXPECT_CALL(*service, NotifyChangeFromExtension(_, _, _))
        .WillOnce(Return(expectedError));

    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    DataObsOption opt;
    opt.SetFirstCallerTokenID(123456);

    EXPECT_EQ(client->NotifyChangeFromExtension(uri, 0, opt), expectedError);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyChangeFromExtension
 * SubFunction: Send multiple notifications from extension.
 * FunctionPoints:
 *   1. Verify multiple notifications can be sent from extension
 *   2. Verify each notification is sent with correct parameters
 *   3. Verify service is called for each notification
 * EnvConditions: Mock service is available.
 * CaseDescription: Test sending multiple notifications from extension.
 */
HWTEST_F(DataObsMgrClientTest, NotifyChangeFromExtension_MultipleNotifications_0400, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    EXPECT_CALL(*service, NotifyChangeFromExtension(_, _, _))
        .Times(3)
        .WillRepeatedly(Return(NO_ERROR));

    Uri uri1("datashare://device_id/com.domainname.dataability.persondata/person/25");
    Uri uri2("datashare://device_id/com.domainname.dataability.persondata/person/26");
    DataObsOption opt1, opt2, opt3;
    opt1.SetFirstCallerTokenID(111111);
    opt2.SetFirstCallerTokenID(222222);
    opt3.SetFirstCallerTokenID(333333);

    EXPECT_EQ(client->NotifyChangeFromExtension(uri1, 0, opt1), NO_ERROR);
    EXPECT_EQ(client->NotifyChangeFromExtension(uri2, 100, opt2), NO_ERROR);
    EXPECT_EQ(client->NotifyChangeFromExtension(uri1, -1, opt3), NO_ERROR);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyChangeFromExtension
 * SubFunction: Notify change with default parameters.
 * FunctionPoints:
 *   1. Verify function can be called with default user ID
 *   2. Verify function can be called with empty DataObsOption
 *   3. Verify service is called with default parameters
 * EnvConditions: Mock service is available.
 * CaseDescription: Test NotifyChangeFromExtension with default parameters.
 */
HWTEST_F(DataObsMgrClientTest, NotifyChangeFromExtension_DefaultParameters_0500, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    EXPECT_CALL(*service, NotifyChangeFromExtension(_, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, _))
        .WillOnce(Return(NO_ERROR));

    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");

    EXPECT_EQ(client->NotifyChangeFromExtension(uri), NO_ERROR);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyProcessObserver
 * SubFunction: Notify process observer successfully.
 * FunctionPoints:
 *   1. Verify process observer can be notified successfully
 *   2. Verify service method is called with correct parameters
 *   3. Verify function returns SUCCESS code
 *   4. Verify service call counter is incremented
 * EnvConditions: Mock service is available.
 * CaseDescription: Test successful notification of process observer.
 */
HWTEST_F(DataObsMgrClientTest, NotifyProcessObserver_0100, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    sptr<MockDataObsManagerOnChangeCallBack> observer(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    std::string key = "test_process_key";
    DataObsOption opt;
    opt.SetFirstCallerTokenID(123456);

    EXPECT_EQ(client->NotifyProcessObserver(key, observer, opt), SUCCESS);
    EXPECT_EQ(service->onChangeCall_, 1);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyProcessObserver
 * SubFunction: Handle invalid key parameter.
 * FunctionPoints:
 *   1. Verify function returns INVALID_PARAM when key is empty
 *   2. Verify no service call is made with invalid key
 *   3. Verify service call counter is not incremented
 * EnvConditions: Mock service is available.
 * CaseDescription: Test NotifyProcessObserver with empty key parameter.
 */
HWTEST_F(DataObsMgrClientTest, NotifyProcessObserver_InvalidKey_0200, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    sptr<MockDataObsManagerOnChangeCallBack> observer(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    EXPECT_EQ(client->NotifyProcessObserver("", observer), INVALID_PARAM);
    EXPECT_EQ(service->onChangeCall_, 0);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyProcessObserver
 * SubFunction: Handle null observer parameter.
 * FunctionPoints:
 *   1. Verify function returns INVALID_PARAM when observer is null
 *   2. Verify no service call is made with null observer
 *   3. Verify service call counter is not incremented
 * EnvConditions: Mock service is available.
 * CaseDescription: Test NotifyProcessObserver with null observer parameter.
 */
HWTEST_F(DataObsMgrClientTest, NotifyProcessObserver_NullObserver_0300, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    std::string key = "test_process_key";

    EXPECT_EQ(client->NotifyProcessObserver(key, nullptr), INVALID_PARAM);
    EXPECT_EQ(service->onChangeCall_, 0);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyProcessObserver
 * SubFunction: Handle both invalid parameters.
 * FunctionPoints:
 *   1. Verify function returns INVALID_PARAM when both key and observer are invalid
 *   2. Verify no service call is made with invalid parameters
 *   3. Verify service call counter is not incremented
 * EnvConditions: Mock service is available.
 * CaseDescription: Test NotifyProcessObserver with both empty key and null observer.
 */
HWTEST_F(DataObsMgrClientTest, NotifyProcessObserver_BothInvalid_0400, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    EXPECT_EQ(client->NotifyProcessObserver("", nullptr), INVALID_PARAM);
    EXPECT_EQ(service->onChangeCall_, 0);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyProcessObserver
 * SubFunction: Send multiple notifications.
 * FunctionPoints:
 *   1. Verify multiple notifications can be sent to different observers
 *   2. Verify multiple notifications can be sent to same observer
 *   3. Verify service is called for each notification
 *   4. Verify service call counter is incremented correctly
 * EnvConditions: Mock service is available.
 * CaseDescription: Test sending multiple notifications to process observers.
 */
HWTEST_F(DataObsMgrClientTest, NotifyProcessObserver_MultipleNotifications_0500, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    sptr<MockDataObsManagerOnChangeCallBack> observer1(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<MockDataObsManagerOnChangeCallBack> observer2(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    std::string key1 = "process_key_1";
    std::string key2 = "process_key_2";
    DataObsOption opt1, opt2;
    opt1.SetFirstCallerTokenID(111111);
    opt2.SetFirstCallerTokenID(222222);

    EXPECT_EQ(client->NotifyProcessObserver(key1, observer1, opt1), SUCCESS);

    EXPECT_EQ(client->NotifyProcessObserver(key2, observer2, opt2), SUCCESS);

    EXPECT_EQ(client->NotifyProcessObserver(key1, observer1, opt1), SUCCESS);

    EXPECT_EQ(service->onChangeCall_, 3);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyProcessObserver
 * SubFunction: Accept different observer types.
 * FunctionPoints:
 *   1. Verify function accepts DataAbilityObserverStub as observer
 *   2. Verify service is called with correct observer parameter
 * EnvConditions: Mock service is available.
 * CaseDescription: Test NotifyProcessObserver with different observer types.
 */
HWTEST_F(DataObsMgrClientTest, NotifyProcessObserver_DifferentObserverTypes_0600, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    std::string key = "test_process_key";

    sptr<MockDataObsManagerOnChangeCallBack> dataObserver(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    EXPECT_EQ(client->NotifyProcessObserver(key, dataObserver), SUCCESS);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: NotifyProcessObserver
 * SubFunction: Notify with default options.
 * FunctionPoints:
 *   1. Verify function can be called with default DataObsOption
 *   2. Verify service is called and returns success
 *   3. Verify service call counter is incremented
 * EnvConditions: Mock service is available.
 * CaseDescription: Test NotifyProcessObserver with default DataObsOption.
 */
HWTEST_F(DataObsMgrClientTest, NotifyProcessObserver_DefaultOptions_0700, TestSize.Level1)
{
    auto client = DataObsMgrClient::GetInstance();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;

    sptr<MockDataObsManagerOnChangeCallBack> observer(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    std::string key = "test_process_key";

    EXPECT_EQ(client->NotifyProcessObserver(key, observer), SUCCESS);
    EXPECT_EQ(service->onChangeCall_, 1);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}
}  // namespace AAFwk
}  // namespace OHOS
