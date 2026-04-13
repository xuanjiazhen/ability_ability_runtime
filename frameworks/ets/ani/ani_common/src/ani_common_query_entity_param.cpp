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

#include "ani_common_query_entity_param.h"

#include "hilog_tag_wrapper.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
} // namespace

static bool UnwrapQueryEntityWantParamters(ani_env *env, ani_object param, InsightIntentQueryParam &queryParam)
{
    if (env == nullptr || param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        return false;
    }

    ani_ref refQueryEntityParam = nullptr;
    if (!GetRefProperty(env, param, "queryEntityParam", refQueryEntityParam)) {
        TAG_LOGE(AAFwkTag::INTENT, "null queryEntityParam");
        return false;
    }

    auto objQueryEntityParam = reinterpret_cast<ani_object>(refQueryEntityParam);
    if (objQueryEntityParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null queryEntityParam");
        return false;
    }

    std::string queryType = "";
    ani_ref aniQueryType = nullptr;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(objQueryEntityParam, "queryType", &aniQueryType)) {
        TAG_LOGE(AAFwkTag::INTENT, "Object_GetField_Ref queryType");
        return false;
    }
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env,
        static_cast<ani_enum_item>(aniQueryType), queryType)) {
        TAG_LOGE(AAFwkTag::INTENT, "GetStdString failed queryType");
        return false;
    }
    queryParam.queryEntityParam_.queryType_ = queryType;

    ani_ref refParameters = nullptr;
    if (!GetRefProperty(env, objQueryEntityParam, "parameters", refParameters)) {
        TAG_LOGE(AAFwkTag::INTENT, "null parameters");
        return false;
    }
    auto wpParam = std::make_shared<WantParams>();
    if (!UnwrapWantParams(env, refParameters, *wpParam)) {
        TAG_LOGE(AAFwkTag::INTENT, "unwrap want fail");
        return false;
    }

    queryParam.queryEntityParam_.parameters_ = wpParam;
    return true;
}

bool UnwrapQueryEntityParam(ani_env *env, ani_object param, InsightIntentQueryParam &queryParam)
{
    if (env == nullptr || param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        return false;
    }

    std::string bundleName = "";
    if (!GetStringProperty(env, param, "bundleName", bundleName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type bundleName");
        return false;
    }
    queryParam.bundleName_ = bundleName;

    std::string moduleName = "";
    if (!GetStringProperty(env, param, "moduleName", moduleName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type moduleName");
        return false;
    }
    queryParam.moduleName_ = moduleName;

    std::string intentName = "";
    if (!GetStringProperty(env, param, "intentName", intentName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type intentName");
        return false;
    }
    queryParam.intentName_ = intentName;

    std::string className = "";
    if (!GetStringProperty(env, param, "className", className)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type className");
        return false;
    }
    queryParam.className_ = className;

    if (IsExistsProperty(env, param, "userId")) {
        ani_int userId = DEFAULT_INVAL_VALUE;
        if (!GetIntPropertyObject(env, param, "userId", userId)) {
            TAG_LOGE(AAFwkTag::INTENT, "Wrong argument userId fail");
            return false;
        }
        queryParam.userId_ = static_cast<int32_t>(userId);
    }

    if (!UnwrapQueryEntityWantParamters(env, param, queryParam)) {
        TAG_LOGE(AAFwkTag::INTENT, "unwrap paramters fail");
        return false;
    }

    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
