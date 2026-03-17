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

#include "js_agent_manager_utils.h"

#include <cstdint>
#include <memory>

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
bool IsValidAgentCardTypeValue(int32_t type)
{
    return type >= static_cast<int32_t>(AgentCardType::APP) &&
        type <= static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE);
}

bool IsObject(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    return napi_typeof(env, value, &type) == napi_ok && type == napi_object;
}

bool GetNamedProperty(napi_env env, napi_value object, const char *name, napi_value &value)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        return false;
    }
    return napi_get_named_property(env, object, name, &value) == napi_ok && value != nullptr;
}

bool ParseRequiredStringProperty(napi_env env, napi_value object, const char *name, std::string &value)
{
    napi_value property = nullptr;
    return GetNamedProperty(env, object, name, property) && ConvertFromJsValue(env, property, value) && !value.empty();
}

bool ParseOptionalStringProperty(napi_env env, napi_value object, const char *name, std::string &value)
{
    napi_value property = nullptr;
    if (!GetNamedProperty(env, object, name, property)) {
        return true;
    }
    return ConvertFromJsValue(env, property, value);
}

bool ParseOptionalBoolProperty(napi_env env, napi_value object, const char *name, bool &value)
{
    napi_value property = nullptr;
    if (!GetNamedProperty(env, object, name, property)) {
        return true;
    }
    return ConvertFromJsValue(env, property, value);
}

bool ParseOptionalInt32Property(napi_env env, napi_value object, const char *name, int32_t &value)
{
    napi_value property = nullptr;
    if (!GetNamedProperty(env, object, name, property)) {
        return true;
    }
    return ConvertFromJsValue(env, property, value);
}

bool ParseStringArray(napi_env env, napi_value value, std::vector<std::string> &out)
{
    bool isArray = false;
    if (napi_is_array(env, value, &isArray) != napi_ok || !isArray) {
        return false;
    }
    uint32_t length = 0;
    if (napi_get_array_length(env, value, &length) != napi_ok) {
        return false;
    }
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        if (napi_get_element(env, value, i, &element) != napi_ok) {
            return false;
        }
        std::string item;
        if (!ConvertFromJsValue(env, element, item) || item.empty()) {
            return false;
        }
        out.emplace_back(item);
    }
    return true;
}
}  // namespace

napi_value CreateJsAgentProvider(napi_env env, const AgentProvider &provider)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create provider");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "organization", CreateJsValue(env, provider.organization));
    napi_set_named_property(env, object, "url", CreateJsValue(env, provider.url));
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentAppInfo(napi_env env, const AgentAppInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create appInfo");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    if (!appInfo.bundleName.empty()) {
        napi_set_named_property(env, object, "bundleName", CreateJsValue(env, appInfo.bundleName));
    }
    if (!appInfo.moduleName.empty()) {
        napi_set_named_property(env, object, "moduleName", CreateJsValue(env, appInfo.moduleName));
    }
    if (!appInfo.abilityName.empty()) {
        napi_set_named_property(env, object, "abilityName", CreateJsValue(env, appInfo.abilityName));
    }
    if (!appInfo.deviceTypes.empty()) {
        napi_set_named_property(env, object, "deviceTypes", CreateNativeArray(env, appInfo.deviceTypes));
    }
    if (!appInfo.minAppVersion.empty()) {
        napi_set_named_property(env, object, "minAppVersion", CreateJsValue(env, appInfo.minAppVersion));
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentCapabilities(napi_env env, const AgentCapabilities &capabilities)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create capabilities");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "streaming", CreateJsValue(env, capabilities.streaming));
    napi_set_named_property(env, object, "pushNotifications", CreateJsValue(env, capabilities.pushNotifications));
    napi_set_named_property(env, object, "stateTransitionHistory",
        CreateJsValue(env, capabilities.stateTransitionHistory));
    if (!capabilities.extension.empty()) {
        napi_set_named_property(env, object, "extension", CreateJsValue(env, capabilities.extension));
    }
    napi_set_named_property(env, object, "extendedAgentCard", CreateJsValue(env, capabilities.extendedAgentCard));
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentSkill(napi_env env, const AgentSkill &skill)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create skill");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "id", CreateJsValue(env, skill.id));
    napi_set_named_property(env, object, "name", CreateJsValue(env, skill.name));
    napi_set_named_property(env, object, "description", CreateJsValue(env, skill.description));
    napi_set_named_property(env, object, "tags", CreateNativeArray(env, skill.tags));
    napi_set_named_property(env, object, "examples", CreateNativeArray(env, skill.examples));
    napi_set_named_property(env, object, "inputModes", CreateNativeArray(env, skill.inputModes));
    napi_set_named_property(env, object, "outputModes", CreateNativeArray(env, skill.outputModes));
    if (!skill.extension.empty()) {
        napi_set_named_property(env, object, "extension", CreateJsValue(env, skill.extension));
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentSkillArray(napi_env env, const std::vector<std::shared_ptr<AgentSkill>> &skills)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create skill array");
    napi_value object = nullptr;
    napi_status status = napi_create_array_with_length(env, skills.size(), &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }

    uint32_t index = 0;
    for (auto skill : skills) {
        if (skill) {
            napi_set_element(env, object, index++, CreateJsAgentSkill(env, *skill));
        }
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentCard(napi_env env, const AgentCard &card)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create card");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "agentId", CreateJsValue(env, card.agentId));
    napi_set_named_property(env, object, "type", CreateJsValue(env, static_cast<int32_t>(card.type)));
    napi_set_named_property(env, object, "name", CreateJsValue(env, card.name));
    napi_set_named_property(env, object, "description", CreateJsValue(env, card.description));
    napi_set_named_property(env, object, "version", CreateJsValue(env, card.version));
    if (!card.documentationUrl.empty()) {
        napi_set_named_property(env, object, "documentationUrl", CreateJsValue(env, card.documentationUrl));
    }
    napi_set_named_property(env, object, "defaultInputModes", CreateNativeArray(env, card.defaultInputModes));
    napi_set_named_property(env, object, "defaultOutputModes", CreateNativeArray(env, card.defaultOutputModes));
    if (card.provider) {
        napi_set_named_property(env, object, "provider", CreateJsAgentProvider(env, *(card.provider)));
    }
    if (card.capabilities) {
        napi_set_named_property(env, object, "capabilities", CreateJsAgentCapabilities(env, *(card.capabilities)));
    }
    napi_set_named_property(env, object, "skills", CreateJsAgentSkillArray(env, card.skills));
    if (!card.extension.empty()) {
        napi_set_named_property(env, object, "extension", CreateJsValue(env, card.extension));
    }
    if (!card.category.empty()) {
        napi_set_named_property(env, object, "category", CreateJsValue(env, card.category));
    }
    if (!card.iconUrl.empty()) {
        napi_set_named_property(env, object, "iconUrl", CreateJsValue(env, card.iconUrl));
    }
    if (card.appInfo) {
        napi_set_named_property(env, object, "appInfo", CreateJsAgentAppInfo(env, *(card.appInfo)));
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentCardArray(napi_env env, const std::vector<AgentCard> &cards)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create card array");
    napi_value object = nullptr;
    napi_status status = napi_create_array_with_length(env, cards.size(), &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }

    uint32_t index = 0;
    for (const auto &card : cards) {
        napi_set_element(env, object, index++, CreateJsAgentCard(env, card));
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

bool ParseJsAgentProvider(napi_env env, napi_value value, AgentProvider &provider)
{
    if (!IsObject(env, value)) {
        return false;
    }
    return ParseRequiredStringProperty(env, value, "organization", provider.organization) &&
        ParseRequiredStringProperty(env, value, "url", provider.url);
}

bool ParseJsAgentCapabilities(napi_env env, napi_value value, AgentCapabilities &capabilities)
{
    if (!IsObject(env, value)) {
        return false;
    }
    return ParseOptionalBoolProperty(env, value, "streaming", capabilities.streaming) &&
        ParseOptionalBoolProperty(env, value, "pushNotifications", capabilities.pushNotifications) &&
        ParseOptionalBoolProperty(env, value, "stateTransitionHistory", capabilities.stateTransitionHistory) &&
        ParseOptionalBoolProperty(env, value, "extendedAgentCard", capabilities.extendedAgentCard) &&
        ParseOptionalStringProperty(env, value, "extension", capabilities.extension);
}

bool ParseJsAgentSkill(napi_env env, napi_value value, AgentSkill &skill)
{
    if (!IsObject(env, value)) {
        return false;
    }

    napi_value property = nullptr;
    if (!ParseRequiredStringProperty(env, value, "id", skill.id) ||
        !ParseRequiredStringProperty(env, value, "name", skill.name) ||
        !ParseRequiredStringProperty(env, value, "description", skill.description)) {
        return false;
    }
    if (!GetNamedProperty(env, value, "tags", property) || !ParseStringArray(env, property, skill.tags)) {
        return false;
    }
    if (GetNamedProperty(env, value, "examples", property) && !ParseStringArray(env, property, skill.examples)) {
        return false;
    }
    if (GetNamedProperty(env, value, "inputModes", property) && !ParseStringArray(env, property, skill.inputModes)) {
        return false;
    }
    if (GetNamedProperty(env, value, "outputModes", property) &&
        !ParseStringArray(env, property, skill.outputModes)) {
        return false;
    }
    return ParseOptionalStringProperty(env, value, "extension", skill.extension);
}

bool ParseJsAgentAppInfo(napi_env env, napi_value value, AgentAppInfo &appInfo)
{
    if (!IsObject(env, value)) {
        return false;
    }

    napi_value property = nullptr;
    if (!ParseRequiredStringProperty(env, value, "bundleName", appInfo.bundleName) ||
        !ParseRequiredStringProperty(env, value, "abilityName", appInfo.abilityName)) {
        return false;
    }
    if (!ParseOptionalStringProperty(env, value, "moduleName", appInfo.moduleName) ||
        !ParseOptionalStringProperty(env, value, "minAppVersion", appInfo.minAppVersion)) {
        return false;
    }
    if (GetNamedProperty(env, value, "deviceTypes", property) &&
        !ParseStringArray(env, property, appInfo.deviceTypes)) {
        return false;
    }
    return true;
}

bool ParseJsAgentCard(napi_env env, napi_value value, AgentCard &card)
{
    if (!IsObject(env, value)) {
        return false;
    }

    napi_value property = nullptr;
    if (!ParseRequiredStringProperty(env, value, "agentId", card.agentId) ||
        !ParseRequiredStringProperty(env, value, "name", card.name) ||
        !ParseRequiredStringProperty(env, value, "description", card.description) ||
        !ParseRequiredStringProperty(env, value, "version", card.version) ||
        !ParseRequiredStringProperty(env, value, "category", card.category)) {
        return false;
    }
    int32_t typeValue = static_cast<int32_t>(AgentCardType::APP);
    if (!ParseOptionalInt32Property(env, value, "type", typeValue)) {
        return false;
    }
    if (!IsValidAgentCardTypeValue(typeValue)) {
        return false;
    }
    card.type = static_cast<AgentCardType>(typeValue);
    if (!GetNamedProperty(env, value, "defaultInputModes", property) ||
        !ParseStringArray(env, property, card.defaultInputModes)) {
        return false;
    }
    if (!GetNamedProperty(env, value, "defaultOutputModes", property) ||
        !ParseStringArray(env, property, card.defaultOutputModes)) {
        return false;
    }
    if (!GetNamedProperty(env, value, "skills", property)) {
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, property, &isArray) != napi_ok || !isArray) {
        return false;
    }
    uint32_t length = 0;
    if (napi_get_array_length(env, property, &length) != napi_ok) {
        return false;
    }
    for (uint32_t i = 0; i < length; ++i) {
        napi_value skillValue = nullptr;
        if (napi_get_element(env, property, i, &skillValue) != napi_ok) {
            return false;
        }
        auto skill = std::make_shared<AgentSkill>();
        if (!ParseJsAgentSkill(env, skillValue, *skill)) {
            return false;
        }
        card.skills.emplace_back(skill);
    }

    if (GetNamedProperty(env, value, "provider", property)) {
        auto provider = std::make_shared<AgentProvider>();
        if (!ParseJsAgentProvider(env, property, *provider)) {
            return false;
        }
        card.provider = provider;
    }
    if (GetNamedProperty(env, value, "capabilities", property)) {
        auto capabilities = std::make_shared<AgentCapabilities>();
        if (!ParseJsAgentCapabilities(env, property, *capabilities)) {
            return false;
        }
        card.capabilities = capabilities;
    }
    if (!ParseOptionalStringProperty(env, value, "documentationUrl", card.documentationUrl) ||
        !ParseOptionalStringProperty(env, value, "iconUrl", card.iconUrl) ||
        !ParseOptionalStringProperty(env, value, "extension", card.extension)) {
        return false;
    }
    if (!GetNamedProperty(env, value, "appInfo", property)) {
        return false;
    }
    auto appInfo = std::make_shared<AgentAppInfo>();
    if (!ParseJsAgentAppInfo(env, property, *appInfo)) {
        return false;
    }
    card.appInfo = appInfo;
    return true;
}
}  // namespace AgentRuntime
}  // namespace OHOS
