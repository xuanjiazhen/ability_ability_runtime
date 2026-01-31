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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_environment.h"
#include "js_runtime_lite.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

class JsRuntimeLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsRuntimeLiteTest::SetUpTestCase() {}

void JsRuntimeLiteTest::TearDownTestCase() {}

void JsRuntimeLiteTest::SetUp() {}

void JsRuntimeLiteTest::TearDown() {}

/**
 * @tc.name: JsRuntimeLiteTest_001
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, JsRuntimeLiteTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeLiteTest_001 start");
    Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);

    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeLiteTest_001 end");
}

/**
 * @tc.name: JsRuntimeLiteTest_002
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, JsRuntimeLiteTest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeLiteTest_002 start");
    Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);

    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv2 = nullptr;
    err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv2);
    EXPECT_EQ(err, napi_status::napi_create_ark_runtime_only_one_env_per_thread);

    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);

    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv2->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_destroy_ark_runtime_env_not_exist);
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeLiteTest_002 end");
}

/**
 * @tc.name: JsRuntimeLiteTest_003
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, JsRuntimeLiteTest_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeLiteTest_003 start");
    Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    err = JsRuntimeLite::GetInstance().Init(options, env);
    EXPECT_EQ(err, napi_status::napi_ok);

    err = JsRuntimeLite::GetInstance().RemoveJsEnv(env);
    EXPECT_EQ(err, napi_status::napi_ok);
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeLiteTest_003 end");
}

/**
 * @tc.name: GetChildOptions_0100
 * @tc.desc: JsRuntime test for GetChildOptions.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, GetChildOptions_0100, TestSize.Level1)
{
    auto child = JsRuntimeLite::GetInstance().GetChildOptions();
    EXPECT_TRUE(child == nullptr);
}

/**
 * @tc.name: GetPkgContextInfoListMap_0100
 * @tc.desc: JsRuntimeLiteTest test for GetPkgContextInfoListMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, GetPkgContextInfoListMap_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0100 start");

    std::map<std::string, std::string> modulePkgContentMap;
    std::string pkgContentJsonString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
                "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["entry"] = pkgContentJsonString;

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    std::map<std::string, std::vector<std::vector<std::string>>> ret;
    std::map<std::string, std::string> pkgAliasMap;
    JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(modulePkgContentMap, ret, pkgAliasMap);
    std::string expectString = "library:packageName:library:bundleName:";
    expectString += "com.xxx.xxxx:moduleName:library:version:1.0.0:entryPath::isSO:false:";
    auto it = ret.find("entry");
    ASSERT_EQ(it, ret.end());
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0100 end");
}

/**
 * @tc.name: GetPkgContextInfoListMap_0200
 * @tc.desc: JsRuntimeLiteTest test for GetPkgContextInfoListMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, GetPkgContextInfoListMap_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0200 start");

    std::map<std::string, std::string> modulePkgContentMap;
    std::string pkgContentJsonString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
                "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["entry"] = pkgContentJsonString;

    std::string libraryString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
                "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["library"] = libraryString;

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    std::map<std::string, std::vector<std::vector<std::string>>> ret;
    std::map<std::string, std::string> pkgAliasMap;
    JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(modulePkgContentMap, ret, pkgAliasMap);
    std::string expectString = "library:packageName:library:bundleName:";
    expectString += "com.xxx.xxxx:moduleName:library:version:1.0.0:entryPath::isSO:false:";
    auto it = ret.find("entry");
    ASSERT_EQ(it, ret.end());
    auto libraryIt = ret.find("library");
    ASSERT_EQ(libraryIt, ret.end());
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0200 end");
}

/**
 * @tc.name: GetPkgContextInfoListMap_0300
 * @tc.desc: JsRuntimeLiteTest test for GetPkgContextInfoListMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, GetPkgContextInfoListMap_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0300 start");

    std::map<std::string, std::string> modulePkgContentMap;
    std::string pkgContentJsonString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
                "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["entry"] = pkgContentJsonString;

    std::string libraryString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
                "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["library"] = libraryString;

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    std::unordered_map<std::string, std::pair<std::unique_ptr<uint8_t[]>, size_t>> pkgInfoMap;
    JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(modulePkgContentMap, pkgInfoMap);
    std::string expectString = "library:packageName:library:bundleName:";
    expectString += "com.xxx.xxxx:moduleName:library:version:1.0.0:entryPath::isSO:false:";
    auto it = pkgInfoMap.find("entry");
    ASSERT_EQ(it, pkgInfoMap.end());
    auto libraryIt = pkgInfoMap.find("library");
    ASSERT_EQ(libraryIt, pkgInfoMap.end());
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0300 end");
}

/**
 * @tc.name: Init_0100
 * @tc.desc: JsRuntimeLiteTest test for Init.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, Init_0100, TestSize.Level1)
{
    napi_env env = {};
    Options options;
    JsRuntimeLite::GetInstance().envMap_.clear();

    auto ret = JsRuntimeLite::GetInstance().Init(options, env);
    EXPECT_EQ(ret, napi_status::napi_generic_failure);
}

/**
 * @tc.name: Init_0200
 * @tc.desc: JsRuntimeLiteTest test for Init.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, Init_0200, TestSize.Level1)
{
    napi_env env = {};
    Options options;
    auto jsEnv = std::make_shared<JsEnv::JsEnvironment>();
    jsEnv->vm_ = nullptr;
    JsRuntimeLite::GetInstance().envMap_.emplace(env, jsEnv);

    auto ret = JsRuntimeLite::GetInstance().Init(options, env);
    EXPECT_EQ(ret, napi_status::napi_generic_failure);
}

/**
 * @tc.name: AddEnv_0100
 * @tc.desc: JsRuntimeLiteTest test for AddEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, AddEnv_0100, TestSize.Level1)
{
    napi_env env = {};
    auto jsEnv = std::make_shared<JsEnv::JsEnvironment>();
    JsRuntimeLite::GetInstance().threadIds_.clear();
    JsRuntimeLite::GetInstance().envMap_.clear();
    JsRuntimeLite::GetInstance().envMap_.emplace(env, jsEnv);

    auto ret = JsRuntimeLite::GetInstance().AddEnv(env, jsEnv);
    EXPECT_EQ(ret, napi_status::napi_generic_failure);
}

/**
 * @tc.name: GetEcmaVm_0100
 * @tc.desc: JsRuntimeLiteTest test for GetEcmaVm.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, GetEcmaVm_0100, TestSize.Level1)
{
    auto ret = JsRuntimeLite::GetInstance().GetEcmaVm(nullptr);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetJsEnv_0100
 * @tc.desc: JsRuntimeLiteTest test for GetJsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, GetJsEnv_0100, TestSize.Level1)
{
    napi_env env = {};
    JsRuntimeLite::GetInstance().envMap_.clear();
    
    auto ret = JsRuntimeLite::GetInstance().GetJsEnv(env);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: InitLoop_0100
 * @tc.desc: JsRuntimeLiteTest test for InitLoop.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, InitLoop_0100, TestSize.Level1)
{
    auto ret = JsRuntimeLite::GetInstance().InitLoop(nullptr);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ParsePkgContextInfoJsonString_0100
 * @tc.desc: JsRuntimeLiteTest test for ParsePkgContextInfoJsonString.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, ParsePkgContextInfoJsonString_0100, TestSize.Level1)
{
    nlohmann::json itemObject;
    itemObject["key"] = "value";
    std::string key = "key";
    std::vector<std::string> items = {};
    JsRuntimeLite::GetInstance().ParsePkgContextInfoJsonString(itemObject, key, items);
    auto rBeginIt = items.rbegin();
    EXPECT_EQ(*rBeginIt, "value");
}

/**
 * @tc.name: ParsePkgContextInfoJsonString_0200
 * @tc.desc: JsRuntimeLiteTest test for ParsePkgContextInfoJsonString.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, ParsePkgContextInfoJsonString_0200, TestSize.Level1)
{
    nlohmann::json itemObject;
    itemObject["key"] = "value";
    std::string key = "FakeKey";
    std::vector<std::string> items = {};
    JsRuntimeLite::GetInstance().ParsePkgContextInfoJsonString(itemObject, key, items);
    auto rBeginIt = items.rbegin();
    EXPECT_EQ(*rBeginIt, "");
}

/**
 * @tc.name: LoadAotFile_0100
 * @tc.desc: JsRuntimeLiteTest test for LoadAotFile with nullptr jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, LoadAotFile_0100, TestSize.Level1)
{
    Options options;
    options.hapPath = "/test/path";

    // Should not crash with nullptr jsEnv
    JsRuntimeLite::GetInstance().LoadAotFile(options, nullptr);
    SUCCEED();
}

/**
 * @tc.name: LoadAotFile_0200
 * @tc.desc: JsRuntimeLiteTest test for LoadAotFile with empty hapPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, LoadAotFile_0200, TestSize.Level1)
{
    Options options;
    options.hapPath = "";
    auto jsEnv = std::make_shared<JsEnv::JsEnvironment>();

    // Should return early with empty hapPath
    JsRuntimeLite::GetInstance().LoadAotFile(options, jsEnv);
    SUCCEED();
}

/**
 * @tc.name: InitConsoleModule_0100
 * @tc.desc: JsRuntimeLiteTest test for InitConsoleModule with nullptr jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, InitConsoleModule_0100, TestSize.Level1)
{
    // Should not crash with nullptr jsEnv
    JsRuntimeLite::GetInstance().InitConsoleModule(nullptr);
    SUCCEED();
}

/**
 * @tc.name: InitTimerModule_0100
 * @tc.desc: JsRuntimeLiteTest test for InitTimerModule with nullptr jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, InitTimerModule_0100, TestSize.Level1)
{
    // Should not crash with nullptr jsEnv
    JsRuntimeLite::GetInstance().InitTimerModule(nullptr);
    SUCCEED();
}

/**
 * @tc.name: SetModuleLoadChecker_0100
 * @tc.desc: JsRuntimeLiteTest test for SetModuleLoadChecker with nullptr jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, SetModuleLoadChecker_0100, TestSize.Level1)
{
    std::shared_ptr<ModuleCheckerDelegate> delegate = nullptr;

    // Should not crash with nullptr jsEnv
    JsRuntimeLite::GetInstance().SetModuleLoadChecker(delegate, nullptr);
    SUCCEED();
}

/**
 * @tc.name: SetModuleLoadChecker_0200
 * @tc.desc: JsRuntimeLiteTest test for SetModuleLoadChecker with valid jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, SetModuleLoadChecker_0200, TestSize.Level1)
{
    auto jsEnv = std::make_shared<JsEnv::JsEnvironment>();
    std::shared_ptr<ModuleCheckerDelegate> delegate = nullptr;

    // Should not crash with valid jsEnv
    JsRuntimeLite::GetInstance().SetModuleLoadChecker(delegate, jsEnv);
    SUCCEED();
}

/**
 * @tc.name: SetRequestAotCallback_0100
 * @tc.desc: JsRuntimeLiteTest test for SetRequestAotCallback with nullptr jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, SetRequestAotCallback_0100, TestSize.Level1)
{
    // Should not crash with nullptr jsEnv
    JsRuntimeLite::GetInstance().SetRequestAotCallback(nullptr);
    SUCCEED();
}

/**
 * @tc.name: SetRequestAotCallback_0200
 * @tc.desc: JsRuntimeLiteTest test for SetRequestAotCallback with valid jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, SetRequestAotCallback_0200, TestSize.Level1)
{
    auto jsEnv = std::make_shared<JsEnv::JsEnvironment>();

    // Should not crash with valid jsEnv
    JsRuntimeLite::GetInstance().SetRequestAotCallback(jsEnv);
    SUCCEED();
}

/**
 * @tc.name: InitWorkerModule_0100
 * @tc.desc: JsRuntimeLiteTest test for InitWorkerModule with nullptr jsEnv.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, InitWorkerModule_0100, TestSize.Level1)
{
    Options options;

    // Should not crash with nullptr jsEnv
    JsRuntimeLite::GetInstance().InitWorkerModule(options, nullptr);
    SUCCEED();
}

/**
 * @tc.name: InitWorkerModule_0200
 * @tc.desc: JsRuntimeLiteTest test for InitWorkerModule with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, InitWorkerModule_0200, TestSize.Level1)
{
    Options options;

    auto jsEnv = std::make_shared<JsEnv::JsEnvironment>();

    // Should not crash with valid parameters
    JsRuntimeLite::GetInstance().InitWorkerModule(options, jsEnv);
    SUCCEED();
}

/**
 * @tc.name: SetChildOptions_0100
 * @tc.desc: JsRuntimeLiteTest test for SetChildOptions.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, SetChildOptions_0100, TestSize.Level1)
{
    Options options;
    options.bundleName = "com.test.bundle";
    options.moduleName = "entry";

    // Should not crash
    JsRuntimeLite::GetInstance().SetChildOptions(options);

    // Verify childOptions_ is set
    auto childOptions = JsRuntimeLite::GetInstance().GetChildOptions();
    ASSERT_NE(childOptions, nullptr);
    EXPECT_EQ(childOptions->bundleName, "com.test.bundle");
    EXPECT_EQ(childOptions->moduleName, "entry");
}

/**
 * @tc.name: SetChildOptions_0200
 * @tc.desc: JsRuntimeLiteTest test for SetChildOptions with update.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, SetChildOptions_0200, TestSize.Level1)
{
    Options options1;
    options1.bundleName = "com.test.bundle1";
    options1.moduleName = "entry1";

    Options options2;
    options2.bundleName = "com.test.bundle2";
    options2.moduleName = "entry2";

    JsRuntimeLite::GetInstance().SetChildOptions(options1);
    JsRuntimeLite::GetInstance().SetChildOptions(options2);

    auto childOptions = JsRuntimeLite::GetInstance().GetChildOptions();
    ASSERT_NE(childOptions, nullptr);
    // Should be updated to the latest values
    EXPECT_EQ(childOptions->bundleName, "com.test.bundle2");
    EXPECT_EQ(childOptions->moduleName, "entry2");
}

/**
 * @tc.name: ParsePkgContextInfoJson_0100
 * @tc.desc: JsRuntimeLiteTest test for ParsePkgContextInfoJson with valid json.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, ParsePkgContextInfoJson_0100, TestSize.Level1)
{
    std::string jsonStr = R"({
        "library": {
            "packageName": "library",
            "bundleName": "com.xxx.xxxx",
            "moduleName": "library",
            "version": "1.0.0",
            "entryPath": "/index",
            "isSO": false,
            "dependencyAlias": "libAlias"
        }
    })";

    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr);
    std::vector<std::vector<std::string>> pkgContextInfoList;
    std::map<std::string, std::string> pkgAliasMap;

    JsRuntimeLite::GetInstance().ParsePkgContextInfoJson(jsonObject, pkgContextInfoList, pkgAliasMap);

    EXPECT_EQ(pkgContextInfoList.size(), 1);
    EXPECT_EQ(pkgAliasMap.size(), 1);
    auto aliasIt = pkgAliasMap.find("libAlias");
    EXPECT_NE(aliasIt, pkgAliasMap.end());
    EXPECT_EQ(aliasIt->second, "library");
}

/**
 * @tc.name: ParsePkgContextInfoJson_0200
 * @tc.desc: JsRuntimeLiteTest test for ParsePkgContextInfoJson with isSO=true.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, ParsePkgContextInfoJson_0200, TestSize.Level1)
{
    std::string jsonStr = R"({
        "library": {
            "packageName": "library",
            "bundleName": "com.xxx.xxxx",
            "moduleName": "library",
            "version": "1.0.0",
            "entryPath": "/index",
            "isSO": true
        }
    })";

    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr);
    std::vector<std::vector<std::string>> pkgContextInfoList;
    std::map<std::string, std::string> pkgAliasMap;

    JsRuntimeLite::GetInstance().ParsePkgContextInfoJson(jsonObject, pkgContextInfoList, pkgAliasMap);

    EXPECT_EQ(pkgContextInfoList.size(), 1);

    // Find isSO field value
    auto it = std::find(pkgContextInfoList[0].begin(), pkgContextInfoList[0].end(), "isSO");
    if (it != pkgContextInfoList[0].end() && ++it != pkgContextInfoList[0].end()) {
        EXPECT_EQ(*it, "true");
    }
}

/**
 * @tc.name: ParsePkgContextInfoJson_0300
 * @tc.desc: JsRuntimeLiteTest test for ParsePkgContextInfoJson with null packageName.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, ParsePkgContextInfoJson_0300, TestSize.Level1)
{
    std::string jsonStr = R"({
        "library": {
            "bundleName": "com.xxx.xxxx",
            "moduleName": "library",
            "version": "1.0.0"
        }
    })";

    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr);
    std::vector<std::vector<std::string>> pkgContextInfoList;
    std::map<std::string, std::string> pkgAliasMap;

    JsRuntimeLite::GetInstance().ParsePkgContextInfoJson(jsonObject, pkgContextInfoList, pkgAliasMap);

    EXPECT_EQ(pkgContextInfoList.size(), 1);
    // packageName should be empty string when null
    auto it = std::find(pkgContextInfoList[0].begin(), pkgContextInfoList[0].end(), "packageName");
    if (it != pkgContextInfoList[0].end() && ++it != pkgContextInfoList[0].end()) {
        EXPECT_EQ(*it, "");
    }
}

/**
 * @tc.name: ParsePkgContextInfoJson_0400
 * @tc.desc: JsRuntimeLiteTest test for ParsePkgContextInfoJson with invalid itemObject.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, ParsePkgContextInfoJson_0400, TestSize.Level1)
{
    std::string jsonStr = R"({
        "library": "invalid_string_value"
    })";

    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr);
    std::vector<std::vector<std::string>> pkgContextInfoList;
    std::map<std::string, std::string> pkgAliasMap;

    JsRuntimeLite::GetInstance().ParsePkgContextInfoJson(jsonObject, pkgContextInfoList, pkgAliasMap);

    // Should return early when itemObject is not an object
    EXPECT_EQ(pkgContextInfoList.size(), 0);
}

/**
 * @tc.name: RemoveJsEnv_0100
 * @tc.desc: JsRuntimeLiteTest test for RemoveJsEnv with non-existent env.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, RemoveJsEnv_0100, TestSize.Level1)
{
    napi_env env = nullptr;
    JsRuntimeLite::GetInstance().envMap_.clear();
    JsRuntimeLite::GetInstance().threadIds_.clear();

    auto ret = JsRuntimeLite::GetInstance().RemoveJsEnv(env);
    EXPECT_EQ(ret, napi_status::napi_destroy_ark_runtime_env_not_exist);
}

/**
 * @tc.name: GetJsEnv_0200
 * @tc.desc: JsRuntimeLiteTest test for GetJsEnv with existing env.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeLiteTest, GetJsEnv_0200, TestSize.Level1)
{
    napi_env env = nullptr;
    auto jsEnv = std::make_shared<JsEnv::JsEnvironment>();
    JsRuntimeLite::GetInstance().envMap_.clear();
    JsRuntimeLite::GetInstance().threadIds_.clear();
    JsRuntimeLite::GetInstance().envMap_.emplace(env, jsEnv);

    auto ret = JsRuntimeLite::GetInstance().GetJsEnv(env);
    EXPECT_NE(ret, nullptr);
}

}  // namespace AbilityRuntime
}  // namespace OHOS