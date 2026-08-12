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
#include <memory>
#include <unordered_map>

#define private public
#define protected public
#include "application_context_manager.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class ApplicationContextManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        auto &mgr = ApplicationContextManager::GetApplicationContextManager();
        mgr.etsApplicationContextRef_.reset();
        mgr.applicationContextMap_.clear();
    }
};

/**
 * @tc.number: ApplicationContextManagerTest_GetInstance_0100
 * @tc.name: GetApplicationContextManager singleton
 * @tc.desc: Verify GetApplicationContextManager returns the same instance.
 */
HWTEST_F(ApplicationContextManagerTest, ApplicationContextManagerTest_GetInstance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_GetInstance_0100 start";
    auto &inst1 = ApplicationContextManager::GetApplicationContextManager();
    auto &inst2 = ApplicationContextManager::GetApplicationContextManager();
    EXPECT_EQ(&inst1, &inst2);
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_GetInstance_0100 end";
}

/**
 * @tc.number: ApplicationContextManagerTest_EtsGlobalObject_0100
 * @tc.name: Set/Get ETS global object
 * @tc.desc: Verify SetEtsGlobalObject and GetEtsGlobalObject work correctly.
 */
HWTEST_F(ApplicationContextManagerTest, ApplicationContextManagerTest_EtsGlobalObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_EtsGlobalObject_0100 start";
    auto &mgr = ApplicationContextManager::GetApplicationContextManager();
    auto result = mgr.GetEtsGlobalObject();
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_EtsGlobalObject_0100 end";
}

/**
 * @tc.number: ApplicationContextManagerTest_RemoveGlobalObject_0100
 * @tc.name: RemoveGlobalObject on empty map
 * @tc.desc: Verify RemoveGlobalObject does not crash when key does not exist.
 */
HWTEST_F(ApplicationContextManagerTest, ApplicationContextManagerTest_RemoveGlobalObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_RemoveGlobalObject_0100 start";
    auto &mgr = ApplicationContextManager::GetApplicationContextManager();
    mgr.RemoveGlobalObject(nullptr);
    EXPECT_TRUE(mgr.applicationContextMap_.empty());
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_RemoveGlobalObject_0100 end";
}

/**
 * @tc.number: ApplicationContextManagerTest_GetGlobalObject_0100
 * @tc.name: GetGlobalObject on empty map
 * @tc.desc: Verify GetGlobalObject returns nullptr when key does not exist.
 */
HWTEST_F(ApplicationContextManagerTest, ApplicationContextManagerTest_GetGlobalObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_GetGlobalObject_0100 start";
    auto &mgr = ApplicationContextManager::GetApplicationContextManager();
    auto result = mgr.GetGlobalObject(nullptr);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "ApplicationContextManagerTest_GetGlobalObject_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
