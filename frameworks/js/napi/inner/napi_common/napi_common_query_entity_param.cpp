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

#include "napi_common_query_entity_param.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool UnwrapQueryEntityWantParamters(napi_env env, napi_value param, InsightIntentQueryEntityParam &queryParam)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid params");
        return false;
    }

    napi_value napiIntentParam = nullptr;
    napi_get_named_property(env, param, "queryEntityParam", &napiIntentParam);
    if (napiIntentParam == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null queryEntityParam");
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiIntentParam, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong argument type queryEntityParam");
        return false;
    }

    auto wp = std::make_shared<WantParams>();
    if (!AppExecFwk::UnwrapWantParams(env, napiIntentParam, *wp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap want fail");
        return false;
    }
    queryParam.queryEntityParam_ = wp;
    return true;
}

bool UnwrapQueryEntityParam(napi_env env, napi_value param, InsightIntentQueryEntityParam &queryParam)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid params");
        return false;
    }

    std::string bundleName {""};
    if (!UnwrapStringByPropertyName(env, param, "bundleName", bundleName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type bundleName");
        return false;
    }
    queryParam.bundleName_ = bundleName;

    std::string moduleName {""};
    if (!UnwrapStringByPropertyName(env, param, "moduleName", moduleName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type moduleName");
        return false;
    }
    queryParam.moduleName_ = moduleName;

    std::string intentName {""};
    if (!UnwrapStringByPropertyName(env, param, "intentName", intentName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type intentName");
        return false;
    }
    queryParam.intentName_ = intentName;

    std::string className {""};
    if (!UnwrapStringByPropertyName(env, param, "className", className)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type className");
        return false;
    }
    queryParam.className_ = className;

    if (IsExistsByPropertyName(env, param, "userId")) {
        int32_t userId = DEFAULT_INVAL_VALUE;
        if (!UnwrapInt32ByPropertyName(env, param, "userId", userId)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument userId fail");
            return false;
        }
        queryParam.userId_ = userId;
    }

    if (!UnwrapQueryEntityWantParamters(env, param, queryParam)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap paramters fail");
        return false;
    }

    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
