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

#include "cwant.h"
#include <map>
#include <string>
#include <unistd.h>
#include "hilog_tag_wrapper.h"
#include "want.h"

struct AbilityRuntime_Want {
    AbilityRuntime_Element element;
    std::map<std::string, std::string> params;
    std::map<std::string, int32_t> fds;
};

AbilityRuntime_Want* OH_AbilityRuntime_NewWant(AbilityRuntime_Element element)
{
    std::unique_ptr<AbilityRuntime_Want> want = std::make_unique<AbilityRuntime_Want>();
    want->element.bundleName = element.bundleName;
    want->element.moduleName = element.moduleName;
    want->element.abilityName = element.abilityName;
    return want.release();
}

/**
 *  todo
 */
Want_ErrorCode OH_AbilityRuntime_DeleteWant(AbilityRuntime_Want* want)
{
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null want");
        return WANT_ERROR_CODE_PARAM_INVALID;
    }

    if (!want->params.empty()) {
        want->params.clear();
    }

    for(auto it : want->fds) {
        if(close(it.second) == -1) {
            TAG_LOGE(AAFwkTag::APPKIT, "close fd failed, key: %{public}s, value: %{public}d",
                it.first.c_str(), it.second);
            return WANT_ERROR_CODE_CLOSE_FD_ERROR;
        }
    }

    delete want;
    want = nullptr;
    return WANT_ERROR_CODE_NO_ERROR;
}

/**
 * 
 */
Want_ErrorCode OH_AbilityRuntime_SetWantElement(AbilityRuntime_Want* want, AbilityRuntime_Element element)
{
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null want");
        return WANT_ERROR_CODE_PARAM_INVALID;
    }
    want->element.bundleName = element.bundleName;
    want->element.moduleName = element.moduleName;
    want->element.abilityName = element.abilityName;
    return WANT_ERROR_CODE_NO_ERROR;
}

/**
 * 
 */
Want_ErrorCode OH_AbilityRuntime_GetWantElement(AbilityRuntime_Want* want, AbilityRuntime_Element* element)
{
    if (want == nullptr || element == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null arg");
        return WANT_ERROR_CODE_PARAM_INVALID;
    }
    element->bundleName = want->element.bundleName;
    element->moduleName = want->element.moduleName;
    element->abilityName = want->element.abilityName;
    return WANT_ERROR_CODE_NO_ERROR;
}

/**
 * 
 */
Want_ErrorCode OH_AbilityRuntime_SetWantCharParam(AbilityRuntime_Want* want, const char* key, const char* value)
{
    if (want == nullptr || key == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null want");
        return WANT_ERROR_CODE_PARAM_INVALID;
    }
    want->params[std::string(key)] = std::string(value);
    return WANT_ERROR_CODE_NO_ERROR;
}

/**
 * 
 */
Want_ErrorCode OH_AbilityRuntime_GetWantCharParam(AbilityRuntime_Want* want, const char* key, char* value, size_t valueSize)
{
    if (want == nullptr || key == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null want");
        return WANT_ERROR_CODE_PARAM_INVALID;
    }
    auto it = want->params.find(key);
    if (it == want->params.end()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not found key");
        return WANT_ERROR_CODE_KEY_NOT_FOUND;
    }
    const std::string& foundValue = it->second;
    if (foundValue.size() >= valueSize) {
        TAG_LOGE(AAFwkTag::APPKIT, "no enough buffer");
        return WANT_ERROR_CODE_BUFFER_TOO_SMALL;
    }
    strcpy(value, it->second.c_str());
    return WANT_ERROR_CODE_NO_ERROR;
}

/**
 * 
 */
Want_ErrorCode OH_AbilityRuntime_AddWantFd(AbilityRuntime_Want* want, char* key, int32_t fd)
{
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null want");
        return WANT_ERROR_CODE_PARAM_INVALID;
    }
    want->fds.emplace(std::string(key), fd);
    return WANT_ERROR_CODE_NO_ERROR;
}
