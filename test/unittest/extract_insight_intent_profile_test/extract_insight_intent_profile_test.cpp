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

#include "insight_intent/extract_insight_intent_profile.cpp"
#include "extract_insight_intent_profile.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    const std::string errProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\",\"12345654321\"],"
            "\"intentName\": \"123\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"uri\": \"/data/app/base\","
            "\"example\": \"exampleAAA\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLinkErr\","
            "\"llmDescription\": \"123111321\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"type\": \"object\","
              "\"items\": {"
                "\"type\": \"array\","
                "\"items\": {"
                  "\"propertyNames\": {"
                    "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                  "},"
                  "\"type\": \"object\","
                  "\"required\": [\"entityId\"],"
                  "\"properties\": {"
                    "\"gameType\": {"
                      "\"description\": \"游戏类型\","
                      "\"type\": \"string\","
                      "\"enum\": [\"3D\",\"2D\",\"RPG\"]"
                    "},"
                    "\"entityId\": {"
                      "\"description\": \"游戏唯一实体 id\","
                      "\"type\": \"string\""
                    "},"
                    "\"entityGroupId\": {"
                      "\"description\": \"用于确定游戏的更新形式（每日游戏）\","
                      "\"type\": \"string\""
                    "}"
                  "}"
                "}"
              "}"
            "}"
        "},"
        "{"
            "\"displayDescription\": \"music\","
            "\"schema\": \"ControlPlayback\","
            "\"keywords\": [\"ControlPlayback\"],"
            "\"intentName\": \"123\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"example\": \"exampleBBB\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLinkErr2\","
            "\"llmDescription\": \"播放音乐控制\","
            "\"domain\": \"control\","
            "\"intentVersion\": \"1.0.1\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"oneOf\": ["
                "{"
                  "\"required\": [\"playbackSpeed\"]"
                "},"
                "{"
                  "\"required\": [\"playbackProgress\"]"
                "}"
              "],"
              "\"propertyNames\": {"
                "\"enum\": [\"playbackSpeed\",\"playbackProgress\"]"
              "},"
              "\"type\": \"object\","
              "\"properties\": {"
                "\"playbackSpeed\": {"
                  "\"description\": \"播放倍速\","
                  "\"type\": \"number\","
                  "\"enum\": [0.5,0.75,1,1.25,1.5,2]"
                "},"
                "\"playbackProgress\": {"
                  "\"description\": \"播放进度,单位秒\","
                  "\"type\": \"number\""
                "}"
              "}"
            "}"
        "}"
        "]"
    "}";

    const std::string profileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\",\"12345654321\"],"
            "\"intentName\": \"123\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"uri\": \"/data/app/base\","
            "\"example\": \"exampleAAA\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"123111321\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"type\": \"object\","
              "\"items\": {"
                "\"type\": \"array\","
                "\"items\": {"
                  "\"propertyNames\": {"
                    "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                  "},"
                  "\"type\": \"object\","
                  "\"required\": [\"entityId\"],"
                  "\"properties\": {"
                    "\"gameType\": {"
                      "\"description\": \"游戏类型\","
                      "\"type\": \"string\","
                      "\"enum\": [\"3D\",\"2D\",\"RPG\"]"
                    "},"
                    "\"entityId\": {"
                      "\"description\": \"游戏唯一实体 id\","
                      "\"type\": \"string\""
                    "},"
                    "\"entityGroupId\": {"
                      "\"description\": \"用于确定游戏的更新形式（每日游戏）\","
                      "\"type\": \"string\""
                    "}"
                  "}"
                "}"
              "}"
            "},"
            "\"result\": {"
              "\"type\": \"object\","
              "\"items\": {"
                "\"type\": \"array\","
                "\"items\": {"
                  "\"propertyNames\": {"
                    "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                  "},"
                  "\"type\": \"object\","
                  "\"required\": [\"entityId\"],"
                  "\"properties\": {"
                    "\"gameType\": {"
                      "\"description\": \"游戏类型\","
                      "\"type\": \"string\","
                      "\"enum\": [\"3D\",\"2D\",\"RPG\"]"
                    "},"
                    "\"entityId\": {"
                      "\"description\": \"游戏唯一实体 id\","
                      "\"type\": \"string\""
                    "},"
                    "\"entityGroupId\": {"
                      "\"description\": \"用于确定游戏的更新形式（每日游戏）\","
                      "\"type\": \"string\""
                    "}"
                  "}"
                "}"
              "}"
            "},"
            "\"entities\": ["
              "{"
                "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
                "\"className\": \"SongPlayState\","
                "\"decoratorType\": \"@IntentEntityDecorator\","
                "\"entityId\": \"11\","
                "\"entityCategory\": \"entity Category\","
                "\"parentClassName\": \"base\","
                "\"parameters\": {"
                  "\"type\": \"object\","
                  "\"items\": {"
                    "\"type\": \"array\","
                    "\"items\": {"
                      "\"propertyNames\": {"
                        "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                      "},"
                    "\"type\": \"object\","
                    "\"required\": [\"entityId\"]"
                    "}"
                  "}"
                "}"
              "},"
              "{"
                "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
                "\"className\": \"base\","
                "\"decoratorType\": \"@IntentEntityDecorator\","
                "\"entityId\": \"12\","
                "\"entityCategory\": \"entity1 Category\","
                "\"parameters\": {"
                  "\"type\": \"object\","
                  "\"items\": {"
                    "\"type\": \"array\","
                    "\"items\": {"
                      "\"propertyNames\": {"
                        "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                      "},"
                      "\"type\": \"object\","
                      "\"required\": [\"entityId\"]"
                    "}"
                  "}"
                "}"
              "}"
            "]"
        "},"
        "{"
            "\"displayDescription\": \"music\","
            "\"schema\": \"ControlPlayback\","
            "\"keywords\": [\"ControlPlayback\"],"
            "\"intentName\": \"InsightIntent2\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"uri\": \"/data/app/base\","
            "\"example\": \"exampleBBB\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"播放音乐控制\","
            "\"domain\": \"control\","
            "\"intentVersion\": \"1.0.1\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"oneOf\": ["
                "{"
                  "\"required\": [\"playbackSpeed\"]"
                "},"
                "{"
                  "\"required\": [\"playbackProgress\"]"
                "}"
              "],"
              "\"propertyNames\": {"
                "\"enum\": [\"playbackSpeed\",\"playbackProgress\"]"
              "},"
              "\"type\": \"object\","
              "\"properties\": {"
                "\"playbackSpeed\": {"
                  "\"description\": \"播放倍速\","
                  "\"type\": \"number\","
                  "\"enum\": [0.5,0.75,1,1.25,1.5,2]"
                "},"
                "\"playbackProgress\": {"
                  "\"description\": \"播放进度,单位秒\","
                  "\"type\": \"number\""
                "}"
              "}"
            "},"
            "\"result\": {"
              "\"oneOf\": ["
                "{"
                  "\"required\": [\"playbackSpeed\"]"
                "},"
                "{"
                  "\"required\": [\"playbackProgress\"]"
                "}"
              "],"
              "\"propertyNames\": {"
                "\"enum\": [\"playbackSpeed\",\"playbackProgress\"]"
              "},"
              "\"type\": \"object\","
              "\"properties\": {"
                "\"playbackSpeed\": {"
                  "\"description\": \"播放倍速\","
                  "\"type\": \"number\","
                  "\"enum\": [0.5,0.75,1,1.25,1.5,2]"
                "},"
                "\"playbackProgress\": {"
                  "\"description\": \"播放进度,单位秒\","
                  "\"type\": \"number\""
                "}"
              "}"
            "}"
        "}"
        "]"
    "}";
}

class ExtractInsightIntentProfileTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtractInsightIntentProfileTest::SetUpTestCase()
{}

void ExtractInsightIntentProfileTest::TearDownTestCase()
{}

void ExtractInsightIntentProfileTest::SetUp()
{}

void ExtractInsightIntentProfileTest::TearDown()
{}

/**
 * @tc.number: TransformTo_0100
 * @tc.name: TransformTo
 * @tc.desc: Test TransformTo invalid param profileStr.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0100, TestSize.Level0)
{
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(errProfileJsonStr, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_0200
 * @tc.name: TransformTo, ToJson, ProfileInfoFormat
 * @tc.desc: Test TransformTo profileStr success.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_0200 called. start");
    TAG_LOGI(AAFwkTag::TEST, "profileJsonStr: %{public}s", profileJsonStr.c_str());
    ExtractInsightIntentProfileInfoVec profileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(profileJsonStr, profileInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(profileInfos.insightIntents.size(), 2);
    EXPECT_EQ(profileInfos.insightIntents[0].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos.insightIntents[0].intentName, "123");
    EXPECT_EQ(profileInfos.insightIntents[0].example, "exampleAAA");
    EXPECT_NE(profileInfos.insightIntents[0].result, "");
    EXPECT_EQ(profileInfos.insightIntents[1].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos.insightIntents[1].intentName, "InsightIntent2");
    EXPECT_EQ(profileInfos.insightIntents[1].example, "exampleBBB");
    EXPECT_NE(profileInfos.insightIntents[1].result, "");

    EXPECT_EQ(profileInfos.insightIntents[0].entities.size(), 2);
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].className, "SongPlayState");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].entityId, "11");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].parentClassName, "base");
    EXPECT_NE(profileInfos.insightIntents[0].entities[0].parameters, "");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].className, "base");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].entityId, "12");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].parentClassName, "");
    EXPECT_NE(profileInfos.insightIntents[0].entities[1].parameters, "");
    EXPECT_EQ(profileInfos.insightIntents[1].entities.size(), 0);

    nlohmann::json jsonObject1;
    result = ExtractInsightIntentProfile::ToJson(profileInfos.insightIntents[0], jsonObject1);
    EXPECT_EQ(result, true);
    ExtractInsightIntentProfileInfoVec profileInfos1;
    result = ExtractInsightIntentProfile::TransformTo(jsonObject1.dump(), profileInfos1);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "jsonObject1 dump: %{public}s", jsonObject1.dump().c_str());
    EXPECT_EQ(profileInfos1.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos1.insightIntents[0].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos1.insightIntents[0].intentName, "123");
    EXPECT_EQ(profileInfos1.insightIntents[0].example, "exampleAAA");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities.size(), 2);
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].className, "SongPlayState");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].entityId, "11");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].parentClassName, "base");
    EXPECT_NE(profileInfos1.insightIntents[0].entities[0].parameters, "");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].className, "base");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].entityId, "12");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].parentClassName, "");
    EXPECT_NE(profileInfos1.insightIntents[0].entities[1].parameters, "");

    nlohmann::json jsonObject2;
    result = ExtractInsightIntentProfile::ToJson(profileInfos.insightIntents[1], jsonObject2);
    EXPECT_EQ(result, true);
    ExtractInsightIntentProfileInfoVec profileInfos2;
    result = ExtractInsightIntentProfile::TransformTo(jsonObject2.dump(), profileInfos2);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "jsonObject2 dump: %{public}s", jsonObject2.dump().c_str());
    EXPECT_EQ(profileInfos2.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos2.insightIntents[0].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos2.insightIntents[0].intentName, "InsightIntent2");
    EXPECT_EQ(profileInfos2.insightIntents[0].example, "exampleBBB");
    EXPECT_EQ(profileInfos2.insightIntents[0].entities.size(), 0);

    ExtractInsightIntentInfo info1;
    result = ExtractInsightIntentProfile::ProfileInfoFormat(profileInfos1.insightIntents[0], info1);
    EXPECT_EQ(result, true);
    EXPECT_EQ(info1.domain, "game");
    EXPECT_NE(info1.result, "");
    EXPECT_EQ(info1.entities.size(), 2);
    EXPECT_EQ(info1.entities[0].className, "SongPlayState");
    EXPECT_EQ(info1.entities[0].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(info1.entities[0].entityId, "11");
    EXPECT_EQ(info1.entities[0].parentClassName, "base");
    EXPECT_NE(info1.entities[0].parameters, "");
    EXPECT_EQ(info1.entities[1].className, "base");
    EXPECT_EQ(info1.entities[1].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(info1.entities[1].entityId, "12");
    EXPECT_EQ(info1.entities[1].parentClassName, "");
    EXPECT_NE(info1.entities[1].parameters, "");
    EXPECT_EQ(info1.genericInfo.decoratorType, "@InsightIntentLink");
    InsightIntentLinkInfo linkInfo1 = info1.genericInfo.get<InsightIntentLinkInfo>();
    EXPECT_EQ(linkInfo1.uri, "/data/app/base");
    EXPECT_EQ(linkInfo1.paramMapping.size(), 1);

    ExtractInsightIntentInfo info2;
    result = ExtractInsightIntentProfile::ProfileInfoFormat(profileInfos2.insightIntents[0], info2);
    EXPECT_EQ(result, true);
    EXPECT_EQ(info2.domain, "control");
    EXPECT_NE(info2.result, "");
    EXPECT_EQ(info2.entities.size(), 0);
    EXPECT_EQ(info2.genericInfo.decoratorType, "@InsightIntentLink");
    InsightIntentLinkInfo linkInfo2 = info2.genericInfo.get<InsightIntentLinkInfo>();
    EXPECT_EQ(linkInfo2.uri, "/data/app/base");
    EXPECT_EQ(linkInfo2.paramMapping.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_0200 called. end");
}

/**
 * @tc.number: TransformTo_0300
 * @tc.name: Test DecoratorType DECORATOR_PAGE branch
 * @tc.desc: Test DECORATOR_PAGE normal and pagePath empty failure
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0300, TestSize.Level0)
{
    // 正常DECORATOR_PAGE场景
    const std::string pageProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"PageIntent\","
            "\"domain\": \"page\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"PageIntentName\","
            "\"pagePath\": \"/pages/TestPage\","
            "\"uiAbility\": \"TestUiAbility\","
            "\"navigationId\": \"nav123\","
            "\"navDestinationName\": \"dest123\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec pageInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageProfileJsonStr, pageInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(pageInfos.insightIntents.size(), 1);
    EXPECT_EQ(pageInfos.insightIntents[0].decoratorType, "@InsightIntentPage");
    EXPECT_EQ(pageInfos.insightIntents[0].pagePath, "/pages/TestPage");

    // PagePath为空的失败场景
    const std::string pageProfileEmptyPathJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"PageIntent\","
            "\"domain\": \"page\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"PageIntentName\","
            "\"pagePath\": \"\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec pageEmptyPathInfos;
    result = ExtractInsightIntentProfile::TransformTo(pageProfileEmptyPathJsonStr, pageEmptyPathInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_0400
 * @tc.name: Test DecoratorType DECORATOR_ENTRY branch
 * @tc.desc: Test DECORATOR_ENTRY normal, abilityName empty failure and unsupported executeMode
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0400, TestSize.Level0)
{
    const std::string entryProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"EntryIntent\","
            "\"domain\": \"entry\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"EntryIntentName\","
            "\"abilityName\": \"TestAbility\","
            "\"executeMode\": [\"foreground\", \"background\"]"
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec entryInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryProfileJsonStr, entryInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(entryInfos.insightIntents.size(), 1);
    EXPECT_EQ(entryInfos.insightIntents[0].decoratorType, "@InsightIntentEntry");
    EXPECT_EQ(entryInfos.insightIntents[0].abilityName, "TestAbility");
}

/**
 * @tc.number: TransformTo_0500
 * @tc.name: Test DecoratorType DECORATOR_FUNCTION branch
 * @tc.desc: Test DECORATOR_FUNCTION normal and functionName empty failure
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0500, TestSize.Level0)
{
    // 正常DECORATOR_FUNCTION场景
    const std::string functionProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentFunction\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"FunctionIntent\","
            "\"domain\": \"function\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"FunctionIntentName\","
            "\"functionName\": \"testFunction\","
            "\"functionParams\": [\"param1\", \"param2\"]"
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec functionInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionProfileJsonStr, functionInfos);
    EXPECT_EQ(result, false);
    EXPECT_EQ(functionInfos.insightIntents.size(), 1);
    EXPECT_EQ(functionInfos.insightIntents[0].decoratorType, "@InsightIntentFunction");
    EXPECT_EQ(functionInfos.insightIntents[0].functionName, "testFunction");

    // functionName为空的失败场景
    const std::string functionEmptyNameJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentFunction\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"FunctionIntent\","
            "\"domain\": \"function\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"FunctionIntentName\","
            "\"functionName\": \"\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec functionEmptyNameInfos;
    result = ExtractInsightIntentProfile::TransformTo(functionEmptyNameJsonStr, functionEmptyNameInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_0600
 * @tc.name: Test DecoratorType DECORATOR_FORM branch
 * @tc.desc: Test DECORATOR_FORM normal and formName/abilityName empty failure
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0600, TestSize.Level0)
{
    // 正常DECORATOR_FORM场景
    const std::string formProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"FormIntent\","
            "\"domain\": \"form\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"FormIntentName\","
            "\"formName\": \"testForm\","
            "\"abilityName\": \"TestAbility\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec formInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(formProfileJsonStr, formInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(formInfos.insightIntents.size(), 1);
    EXPECT_EQ(formInfos.insightIntents[0].decoratorType, "@InsightIntentForm");
    EXPECT_EQ(formInfos.insightIntents[0].formName, "testForm");
    EXPECT_EQ(formInfos.insightIntents[0].abilityName, "TestAbility");
}

/**
 * @tc.number: TransformTo_0700
 * @tc.name: Test DecoratorType DECORATOR_UNKNOWN branch
 * @tc.desc: Test invalid decoratorType failure
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0700, TestSize.Level0)
{
    const std::string unknownDecoratorJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InvalidType\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"UnknownIntent\","
            "\"domain\": \"unknown\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"UnknownIntentName\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec unknownInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(unknownDecoratorJsonStr, unknownInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_0800
 * @tc.name: Test CheckProfileInfo required fields empty
 * @tc.desc: Test failure when required fields are empty
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0800, TestSize.Level0)
{
    // decoratorFile为空
    const std::string emptyDecoratorFileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"EmptyFieldIntent\","
            "\"domain\": \"link\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"EmptyFieldIntentName\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": [{"
                "\"paramName\": \"aaa\""
            "}]"
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec emptyDecoratorFileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(emptyDecoratorFileJsonStr, emptyDecoratorFileInfos);
    EXPECT_EQ(result, false);

    // bundleName为空
    const std::string emptyBundleNameJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"bundleName\": \"\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"EmptyFieldIntent\","
            "\"domain\": \"link\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"EmptyFieldIntentName\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": [{"
                "\"paramName\": \"aaa\""
            "}]"
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec emptyBundleNameInfos;
    result = ExtractInsightIntentProfile::TransformTo(emptyBundleNameJsonStr, emptyBundleNameInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_0900
 * @tc.name: Test entity required fields empty
 * @tc.desc: Test failure when entity className/entityId is empty
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0900, TestSize.Level0)
{
    // entity className为空
    const std::string entityEmptyClassNameJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"EntityEmptyIntent\","
            "\"domain\": \"link\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"EntityEmptyIntentName\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": [{"
                "\"paramName\": \"aaa\""
            "}],"
            "\"entities\": [{"
                "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
                "\"className\": \"\","
                "\"decoratorType\": \"@IntentEntityDecorator\","
                "\"entityId\": \"11\","
                "\"entityCategory\": \"entity Category\""
            "}]"
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec entityEmptyClassNameInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(entityEmptyClassNameJsonStr, entityEmptyClassNameInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_1000
 * @tc.name: Test parameters/result not object type
 * @tc.desc: Test parse failure when parameters/result is not object
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_1000, TestSize.Level0)
{
    const std::string paramsNotObjectJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"ParamsNotObjectIntent\","
            "\"domain\": \"link\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"ParamsNotObjectIntentName\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": [{"
                "\"paramName\": \"aaa\""
            "}],"
            "\"parameters\": [1,2,3]"
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec paramsNotObjectInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(paramsNotObjectJsonStr, paramsNotObjectInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_1100
 * @tc.name: Test parameters/result not object type
 * @tc.desc: Test parse failure when parameters/result is not object
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_1100, TestSize.Level0)
{
    const std::string resultNotObjectJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"ResultNotObjectIntent\","
            "\"domain\": \"link\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"ResultNotObjectIntentName\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": [{"
                "\"paramName\": \"aaa\""
            "}],"
            "\"result\": \"not object\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec resultNotObjectInfos;
    auto result = ExtractInsightIntentProfile::TransformTo(resultNotObjectJsonStr, resultNotObjectInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_1200
 * @tc.name: Test parameters/result not object type
 * @tc.desc: Test parse failure when parameters/result is not object
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_1200, TestSize.Level0)
{
    const std::string entityParamsNotObjectJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"EntityParamsNotObjectIntent\","
            "\"domain\": \"link\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"EntityParamsNotObjectIntentName\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": [{"
                "\"paramName\": \"aaa\""
            "}],"
            "\"entities\": [{"
                "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
                "\"className\": \"TestEntity\","
                "\"decoratorType\": \"@IntentEntityDecorator\","
                "\"entityId\": \"11\","
                "\"entityCategory\": \"entity Category\","
                "\"parameters\": 123"
            "}]"
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec entityParamsNotObjectInfos;
    auto result = ExtractInsightIntentProfile::TransformTo(entityParamsNotObjectJsonStr, entityParamsNotObjectInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_1300
 * @tc.name: Test DecoratorType DECORATOR_FORM branch
 * @tc.desc: Test DECORATOR_FORM normal and formName/abilityName empty failure
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_1300, TestSize.Level0)
{
    const std::string formEmptyFormNameJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"FormIntent\","
            "\"domain\": \"form\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"FormIntentName\","
            "\"formName\": \"\","
            "\"abilityName\": \"TestAbility\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec formEmptyFormNameInfos;
    auto result = ExtractInsightIntentProfile::TransformTo(formEmptyFormNameJsonStr, formEmptyFormNameInfos);
    EXPECT_EQ(result, false);

    // abilityName为空的失败场景
    const std::string formEmptyAbilityNameJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorClass\": \"base\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"bundleName\": \"com.example.instent\","
            "\"moduleName\": \"entry\","
            "\"intentName\": \"FormIntent\","
            "\"domain\": \"form\","
            "\"intentVersion\": \"1.0.0\","
            "\"displayName\": \"FormIntentName\","
            "\"formName\": \"testForm\","
            "\"abilityName\": \"\""
        "}"
        "]"
    "}";

    ExtractInsightIntentProfileInfoVec formEmptyAbilityNameInfos;
    result = ExtractInsightIntentProfile::TransformTo(formEmptyAbilityNameJsonStr, formEmptyAbilityNameInfos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: ToJson_0100
 * @tc.name: Test ToJson parameters/result discarded
 * @tc.desc: Test ToJson failure when parameters/result is invalid json string
 */
HWTEST_F(ExtractInsightIntentProfileTest, ToJson_0100, TestSize.Level0)
{
    // 构造parameters为无效json字符串的info
    ExtractInsightIntentProfileInfo invalidParamsInfo;
    invalidParamsInfo.decoratorFile = "@normalized:N&&&entry/src/main/ets/pages/Index&";
    invalidParamsInfo.decoratorClass = "base";
    invalidParamsInfo.decoratorType = "@InsightIntentLink";
    invalidParamsInfo.bundleName = "com.example.instent";
    invalidParamsInfo.moduleName = "entry";
    invalidParamsInfo.intentName = "InvalidParamsIntent";
    invalidParamsInfo.domain = "link";
    invalidParamsInfo.intentVersion = "1.0.0";
    invalidParamsInfo.displayName = "InvalidParamsIntentName";
    invalidParamsInfo.uri = "/data/app/base";
    invalidParamsInfo.parameters = "{invalid json}";

    nlohmann::json invalidParamsJson;
    bool result = ExtractInsightIntentProfile::ToJson(invalidParamsInfo, invalidParamsJson);
    EXPECT_EQ(result, true);
    EXPECT_EQ(invalidParamsJson.contains(INSIGHT_INTENT_PARAMETERS), false);
}

/**
 * @tc.number: ProfileInfoFormat_0200
 * @tc.name: Test ProfileInfoFormat all decoratorType branches
 * @tc.desc: Test ProfileInfoFormat for PAGE/ENTRY/FUNCTION/FORM decoratorType
 */
HWTEST_F(ExtractInsightIntentProfileTest, ProfileInfoFormat_0200, TestSize.Level0)
{
    ExtractInsightIntentProfileInfo entryProfileInfo;
    entryProfileInfo.decoratorFile = "@normalized:N&&&entry/src/main/ets/pages/Index&";
    entryProfileInfo.decoratorClass = "base";
    entryProfileInfo.decoratorType = "@InsightIntentEntry";
    entryProfileInfo.bundleName = "com.example.instent";
    entryProfileInfo.moduleName = "entry";
    entryProfileInfo.intentName = "EntryFormatIntent";
    entryProfileInfo.domain = "entry";
    entryProfileInfo.intentVersion = "1.0.0";
    entryProfileInfo.displayName = "EntryFormatIntentName";
    entryProfileInfo.abilityName = "TestAbility";
    entryProfileInfo.executeMode = {"foreground"};

    ExtractInsightIntentInfo entryFormatInfo;
    auto result = ExtractInsightIntentProfile::ProfileInfoFormat(entryProfileInfo, entryFormatInfo);
    EXPECT_EQ(result, true);
    EXPECT_EQ(entryFormatInfo.genericInfo.decoratorType, "@InsightIntentEntry");
    InsightIntentEntryInfo entryInfo = entryFormatInfo.genericInfo.get<InsightIntentEntryInfo>();
    EXPECT_EQ(entryInfo.abilityName, "TestAbility");
}

/**
 * @tc.number: ProfileInfoFormat_0300
 * @tc.name: Test ProfileInfoFormat all decoratorType branches
 * @tc.desc: Test ProfileInfoFormat for PAGE/ENTRY/FUNCTION/FORM decoratorType
 */
HWTEST_F(ExtractInsightIntentProfileTest, ProfileInfoFormat_0300, TestSize.Level0)
{
    ExtractInsightIntentProfileInfo formProfileInfo;
    formProfileInfo.decoratorFile = "@normalized:N&&&entry/src/main/ets/pages/Index&";
    formProfileInfo.decoratorClass = "base";
    formProfileInfo.decoratorType = "@InsightIntentForm";
    formProfileInfo.bundleName = "com.example.instent";
    formProfileInfo.moduleName = "entry";
    formProfileInfo.intentName = "FormFormatIntent";
    formProfileInfo.domain = "form";
    formProfileInfo.intentVersion = "1.0.0";
    formProfileInfo.displayName = "FormFormatIntentName";
    formProfileInfo.formName = "testForm";
    formProfileInfo.abilityName = "TestAbility";

    ExtractInsightIntentInfo formFormatInfo;
    auto result = ExtractInsightIntentProfile::ProfileInfoFormat(formProfileInfo, formFormatInfo);
    EXPECT_EQ(result, true);
    EXPECT_EQ(formFormatInfo.genericInfo.decoratorType, "@InsightIntentForm");
    InsightIntentFormInfo formInfo = formFormatInfo.genericInfo.get<InsightIntentFormInfo>();
    EXPECT_EQ(formInfo.formName, "testForm");
    EXPECT_EQ(formInfo.abilityName, "TestAbility");

    ExtractInsightIntentProfileInfo unknownProfileInfo;
    unknownProfileInfo.decoratorFile = "@normalized:N&&&entry/src/main/ets/pages/Index&";
    unknownProfileInfo.decoratorClass = "base";
    unknownProfileInfo.decoratorType = "@InvalidType";
    unknownProfileInfo.bundleName = "com.example.instent";
    unknownProfileInfo.moduleName = "entry";
    unknownProfileInfo.intentName = "UnknownFormatIntent";
    unknownProfileInfo.domain = "unknown";
    unknownProfileInfo.intentVersion = "1.0.0";
    unknownProfileInfo.displayName = "UnknownFormatIntentName";

    ExtractInsightIntentInfo unknownFormatInfo;
    result = ExtractInsightIntentProfile::ProfileInfoFormat(unknownProfileInfo, unknownFormatInfo);
    EXPECT_EQ(result, false);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
