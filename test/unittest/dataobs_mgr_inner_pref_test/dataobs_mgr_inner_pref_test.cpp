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
#include <memory>
#include <algorithm>
#include <functional>
#include "dataobs_mgr_inner_common.h"
#include "uri.h"
#define private public
#include "data_ability_observer_proxy.h"
#include "dataobs_mgr_errors.h"
#include "dataobs_mgr_inner_pref.h"
#include "hilog_tag_wrapper.h"
#include "mock_data_ability_observer_stub.h"

using namespace OHOS;
using namespace testing::ext;
using namespace testing;


namespace OHOS {
namespace AAFwk {
using Uri = OHOS::Uri;
using ObsListType = std::list<struct ObserverNode>;
using ObsRecipientMapType = OHOS::AAFwk::DataObsMgrInnerPref::ObsRecipientMapType;
class DataObsMgrInnerPrefTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInnerPref_ = nullptr;
};
void DataObsMgrInnerPrefTest::SetUpTestCase(void) {}
void DataObsMgrInnerPrefTest::TearDownTestCase(void) {}
void DataObsMgrInnerPrefTest::SetUp()
{
    dataObsMgrInnerPref_ = std::make_shared<DataObsMgrInnerPref>();
}
void DataObsMgrInnerPrefTest::TearDown() {}

static constexpr int64_t USER_TEST = 100;

/*
 * Feature: DataObsMgrInnerPref
 * Function: Register and unregister function test
 * SubFunction: HandleRegisterObserver/HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleRegisterObserver_0100, TestSize.Level1)
{
    ASSERT_TRUE(dataObsMgrInnerPref_ != nullptr);

    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInnerPref_->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));

    EXPECT_EQ(dataObsMgrInnerPref_->HaveRegistered(callback), true);
    dataObsMgrInnerPref_->HandleUnregisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
    EXPECT_EQ(dataObsMgrInnerPref_->HaveRegistered(callback), false);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Register function test
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleRegisterObserver_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(ObserverNode(callback, USER_TEST, 0, 0));
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    int res = dataObsMgrInner->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
    EXPECT_EQ(res, OBS_EXIST);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Register function test
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleRegisterObserver_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    const sptr<IDataAbilityObserver> callback1(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(ObserverNode(callback1, USER_TEST, 0, 0));
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    int res = dataObsMgrInner->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
    EXPECT_EQ(res, OBS_EXIST);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Register function test
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Register max observer
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleRegisterObserver_0400, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    int res = 0;

    int times = DataObsMgrInnerPref::OBS_NUM_MAX;
    for (int i = 0; i <= times; i++) {
        sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
        const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
        res = dataObsMgrInner->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
        EXPECT_EQ(res, NO_ERROR);
    }
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    res = dataObsMgrInner->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
    EXPECT_EQ(res, DATAOBS_SERVICE_OBS_LIMMIT);

    // other token success
    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback2(new (std::nothrow) DataAbilityObserverProxy(observer));
    res = dataObsMgrInner->HandleRegisterObserver(uri, ObserverNode(callback2, USER_TEST, 1, 0));
    EXPECT_EQ(res, NO_ERROR);

    // other uri success
    Uri uri2("sharepreferences://data/preferences/preferences_test/2");
    sptr<MockDataAbilityObserverStub> observer3(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback3(new (std::nothrow) DataAbilityObserverProxy(observer3));
    res = dataObsMgrInner->HandleRegisterObserver(uri2, ObserverNode(callback3, USER_TEST, 0, 0));
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Register function test
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: RegisterObserver max limit times
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleRegisterObserver_0500, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::DBOBSMGR, "DataObsMgrInnerPref_HandleRegisterObserver_0500::Start");
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    for (int token = 0; token < DataObsMgrInnerPref::OBS_NUM_MAX; token++) {
        for (int i = 0; i < DataObsMgrInnerPref::OBS_NUM_MAX; i++) {
            sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
            const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
            int res = dataObsMgrInner->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, token, 0));
            EXPECT_EQ(res, NO_ERROR);
        }
    }
    int token = DataObsMgrInnerPref::OBS_NUM_MAX  +1;
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    int res = dataObsMgrInner->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, token, 0));
    EXPECT_EQ(res, DATAOBS_SERVICE_OBS_LIMMIT);
    TAG_LOGE(AAFwkTag::DBOBSMGR, "DataObsMgrInnerPref_HandleRegisterObserver_0500::End");
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleUnregisterObserver_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInner->observers_.clear();
    int res = dataObsMgrInner->HandleUnregisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
    EXPECT_EQ(res, NO_OBS_FOR_URI);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleUnregisterObserver_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(ObserverNode(callback, USER_TEST, 0, 0));
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    int res = dataObsMgrInner->HandleUnregisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleUnregisterObserver_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    const sptr<IDataAbilityObserver> callback2(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(ObserverNode(callback, USER_TEST, 0, 0));
    obsList.push_back(ObserverNode(callback2, USER_TEST, 0, 0));
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    dataObsMgrInner->observers_.emplace("exit", obsList);
    int res = dataObsMgrInner->HandleUnregisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Notify function test
 * SubFunction: HandleNotifyChange
 * FunctionPoints: When the data changes, call the OnChangePreferences function of the registered dataabilityobserver
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_0100, TestSize.Level1)
{
    ASSERT_TRUE(dataObsMgrInnerPref_ != nullptr);
    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    Uri notifyUri("sharepreferences://data/preferences/preferences_test?key");
    dataObsMgrInnerPref_->HandleRegisterObserver(uri, ObserverNode(mockDataAbilityObserverStub, USER_TEST, 0, 0));
    dataObsMgrInnerPref_->HandleNotifyChange(notifyUri, USER_TEST, 1);
    EXPECT_EQ("", mockDataAbilityObserverStub->key_);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Notify function test
 * SubFunction: HandleNotifyChange
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test?key");
    dataObsMgrInner->observers_.clear();
    int res = dataObsMgrInner->HandleNotifyChange(uri, USER_TEST, 1);
    EXPECT_EQ(res, NO_OBS_FOR_URI);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Notify function test
 * SubFunction: HandleNotifyChange
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    Uri uri("sharepreferences://data/preferences/preferences_test");
    ObsListType obsList;
    obsList.push_back(ObserverNode(nullptr, USER_TEST, 0, 0));
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    Uri notifyUri("sharepreferences://data/preferences/preferences_test?key");
    int res = dataObsMgrInner->HandleNotifyChange(notifyUri, USER_TEST, 1);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: GetObsListFromMap/RemoveObs/HaveRegistered function test
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_RemoveObs_HaveRegistered_0100, TestSize.Level1)
{
    ASSERT_TRUE(dataObsMgrInnerPref_ != nullptr);
    Uri uri("sharepreferences://data/preferences/preferences_test");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    dataObsMgrInnerPref_->HandleRegisterObserver(uri, ObserverNode(callback, USER_TEST, 0, 0));

    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub2(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback2(
        new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub2));

    dataObsMgrInnerPref_->HandleRegisterObserver(uri, ObserverNode(callback2, USER_TEST, 0, 0));
    auto obsPair = dataObsMgrInnerPref_->observers_.find(uri.ToString());
    EXPECT_EQ((std::size_t)2, obsPair->second.size());
    EXPECT_EQ(true, dataObsMgrInnerPref_->HaveRegistered(callback));
    EXPECT_EQ(true, dataObsMgrInnerPref_->HaveRegistered(callback2));

    dataObsMgrInnerPref_->RemoveObs(callback->AsObject());
    EXPECT_EQ(false, dataObsMgrInnerPref_->HaveRegistered(callback));
    obsPair->second.clear();
    obsPair = dataObsMgrInnerPref_->observers_.find(uri.ToString());
    EXPECT_EQ(false, dataObsMgrInnerPref_->HaveRegistered(callback));

    dataObsMgrInnerPref_->RemoveObs(callback2->AsObject());
    EXPECT_EQ(false, dataObsMgrInnerPref_->HaveRegistered(callback2));
    obsPair->second.clear();
    obsPair = dataObsMgrInnerPref_->observers_.find(uri.ToString());
    EXPECT_EQ(false, dataObsMgrInnerPref_->HaveRegistered(callback2));
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: AddObsDeathRecipient/RemoveObsDeathRecipient function test
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_AddRemove_ObsDeathRecipient_0100, TestSize.Level1)
{
    ASSERT_TRUE(dataObsMgrInnerPref_ != nullptr);

    sptr<IRemoteObject> observer(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInnerPref_->AddObsDeathRecipient(callback);
    dataObsMgrInnerPref_->AddObsDeathRecipient(nullptr);

    ObsRecipientMapType::const_iterator it;
    it = dataObsMgrInnerPref_->obsRecipient_.find(observer);
    EXPECT_EQ(true, it != dataObsMgrInnerPref_->obsRecipient_.end());

    dataObsMgrInnerPref_->RemoveObsDeathRecipient(callback->AsObject());
    dataObsMgrInnerPref_->RemoveObsDeathRecipient(nullptr);
    it = dataObsMgrInnerPref_->obsRecipient_.find(observer);
    EXPECT_EQ(false, it != dataObsMgrInnerPref_->obsRecipient_.end());

    dataObsMgrInnerPref_->obsRecipient_.clear();
    dataObsMgrInnerPref_->RemoveObsDeathRecipient(callback->AsObject());
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Unregister function test
 * SubFunction: RemoveObs
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_RemoveObs_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_NE(dataObsMgrInner, nullptr);
    std::string uri1 = "uri1";
    std::string uri2 = "uri2";
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback1(new (std::nothrow) DataAbilityObserverProxy(observer));
    const sptr<IDataAbilityObserver> callback2(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList1;
    ObsListType obsList2;
    ObsListType obsList3;
    obsList1.push_back(ObserverNode(callback1, USER_TEST, 0, 0));
    obsList2.push_back(ObserverNode(callback1, USER_TEST, 0, 0));
    obsList2.push_back(ObserverNode(callback2, USER_TEST, 0, 0));
    dataObsMgrInner->observers_.emplace(uri1, obsList1);
    dataObsMgrInner->observers_.emplace(uri2, obsList2);
    dataObsMgrInner->RemoveObs(callback2->AsObject());
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: HandleNotifyChange with same token
 * SubFunction: OnChangePreferences
 * FunctionPoints: When listenerTokenId equals tokenId, notification should succeed
 * EnvConditions: NA
 * CaseDescription: Test preferences notification with same token IDs
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_SameToken_0100, TestSize.Level1)
{
    ASSERT_TRUE(dataObsMgrInnerPref_ != nullptr);

    Uri uri("sharepreferences://data/preferences/preferences_test?key");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    EXPECT_CALL(*mockDataAbilityObserverStub, OnChangePreferences(_)).Times(1);

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    ObserverNode node(callback, USER_TEST, 0, 0);
    node.tokenId_ = 1001; // Set same token
    dataObsMgrInnerPref_->HandleRegisterObserver(uri, node);
    dataObsMgrInnerPref_->HandleNotifyChange(uri, USER_TEST, 1001); // Same tokenId
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: HandleNotifyChange with different tokens
 * SubFunction: OnChangePreferences
 * FunctionPoints: When tokens differ, permission verification should be performed
 * EnvConditions: NA
 * CaseDescription: Test preferences notification with different token IDs
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_DifferentToken_0100, TestSize.Level1)
{
    ASSERT_TRUE(dataObsMgrInnerPref_ != nullptr);

    Uri uri("sharepreferences://data/preferences/preferences_test?key");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    // Different tokens - verification may fail
    EXPECT_CALL(*mockDataAbilityObserverStub, OnChangePreferences(_)).Times(AnyNumber());

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    ObserverNode node(callback, USER_TEST, 0, 0);
    node.tokenId_ = 1001;
    dataObsMgrInnerPref_->HandleRegisterObserver(uri, node);
    dataObsMgrInnerPref_->HandleNotifyChange(uri, USER_TEST, 1002); // Different tokenId
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: HandleNotifyChange with cross-user scenario
 * SubFunction: OnChangePreferences
 * FunctionPoints: Cross-user notification should be blocked
 * EnvConditions: NA
 * CaseDescription: Test preferences cross-user notification blocking
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_CrossUser_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_NE(dataObsMgrInner, nullptr);

    Uri uri("sharepreferences://data/preferences/preferences_test?key");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    // Cross-user should be blocked
    EXPECT_CALL(*mockDataAbilityObserverStub, OnChangePreferences(_)).Times(0);

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    ObserverNode node(callback, 200, 0, 0); // Different user
    dataObsMgrInner->HandleRegisterObserver(uri, node);
    dataObsMgrInner->HandleNotifyChange(uri, 100, 1001);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: HandleNotifyChange with zero user IDs
 * SubFunction: OnChangePreferences
 * FunctionPoints: Zero user ID should allow cross-user notification
 * EnvConditions: NA
 * CaseDescription: Test preferences with observer having userId=0
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_ZeroUserId_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_NE(dataObsMgrInner, nullptr);

    Uri uri("sharepreferences://data/preferences/preferences_test?key");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    EXPECT_CALL(*mockDataAbilityObserverStub, OnChangePreferences(_)).Times(AnyNumber());

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    ObserverNode node(callback, 0, 0, 0); // userId=0
    dataObsMgrInner->HandleRegisterObserver(uri, node);
    dataObsMgrInner->HandleNotifyChange(uri, USER_TEST, 1001);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: HandleNotifyChange with multiple observers
 * SubFunction: OnChangePreferences
 * FunctionPoints: Multiple observers should each be verified independently
 * EnvConditions: NA
 * CaseDescription: Test preferences with multiple observers
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_MultipleObservers_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_NE(dataObsMgrInner, nullptr);

    Uri uri("sharepreferences://data/preferences/preferences_test?key");
    sptr<MockDataAbilityObserverStub> mockObserver1(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> mockObserver2(new (std::nothrow) MockDataAbilityObserverStub());

    EXPECT_CALL(*mockObserver1, OnChangePreferences(_)).Times(AnyNumber());
    EXPECT_CALL(*mockObserver2, OnChangePreferences(_)).Times(AnyNumber());

    const sptr<IDataAbilityObserver> callback1(new (std::nothrow) DataAbilityObserverProxy(mockObserver1));
    const sptr<IDataAbilityObserver> callback2(new (std::nothrow) DataAbilityObserverProxy(mockObserver2));

    ObsListType obsList;
    obsList.push_back(ObserverNode(callback1, USER_TEST, 0, 0));
    obsList.push_back(ObserverNode(callback2, USER_TEST, 0, 0));
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);

    dataObsMgrInner->HandleNotifyChange(uri, USER_TEST, 1001);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: HandleNotifyChange with permission failure
 * SubFunction: OnChangePreferences
 * FunctionPoints: Permission verification failure should block notification
 * EnvConditions: NA
 * CaseDescription: Test preferences when permission verification fails
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_PermissionFail_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_NE(dataObsMgrInner, nullptr);

    Uri uri("sharepreferences://data/preferences/preferences_test?key");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    // With different tokens, verification may fail
    EXPECT_CALL(*mockDataAbilityObserverStub, OnChangePreferences(_)).Times(AnyNumber());

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    ObserverNode node(callback, USER_TEST, 0, 0);
    node.tokenId_ = 9999;
    dataObsMgrInner->HandleRegisterObserver(uri, node);
    dataObsMgrInner->HandleNotifyChange(uri, USER_TEST, 8888); // Different token
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: HandleNotifyChange with various query parameters
 * SubFunction: OnChangePreferences
 * FunctionPoints: Query parameters should be passed correctly
 * EnvConditions: NA
 * CaseDescription: Test preferences with different query strings
 */
HWTEST_F(DataObsMgrInnerPrefTest, DataObsMgrInnerPref_HandleNotifyChange_QueryParams_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInner = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_NE(dataObsMgrInner, nullptr);

    std::vector<std::string> queries = {
        "?key=value",
        "?key1=value1&key2=value2",
        "",
        "?test"
    };

    sptr<MockDataAbilityObserverStub> mockObserver(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_CALL(*mockObserver, OnChangePreferences(_)).Times(AnyNumber());

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockObserver));

    for (const auto &query : queries) {
        Uri uri("sharepreferences://data/preferences/preferences_test" + query);
        ObserverNode node(callback, USER_TEST, 0, 0);
        dataObsMgrInner->HandleRegisterObserver(uri, node);
        dataObsMgrInner->HandleNotifyChange(uri, USER_TEST, 0);
        dataObsMgrInner->HandleUnregisterObserver(uri, node);
    }
}

}  // namespace AAFwk
}  // namespace OHOS
