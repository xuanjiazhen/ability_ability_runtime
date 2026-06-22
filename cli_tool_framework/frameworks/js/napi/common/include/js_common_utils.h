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

#ifndef OHOS_ABILITY_RUNTIME_JS_COMMON_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_COMMON_UTILS_H

#include <string>
#include <napi/native_api.h>

namespace OHOS {
namespace CliTool {

/**
 * @brief Parse JSON string and convert to JavaScript object
 * @param env The N-API environment
 * @param jsonStr The JSON string to parse
 * @return Returns the JavaScript object
 */
napi_value ParseJsonStringToJsObject(napi_env env, const std::string &jsonStr);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_COMMON_UTILS_H
