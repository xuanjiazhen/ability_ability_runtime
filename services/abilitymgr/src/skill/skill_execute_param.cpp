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

#include "skill_execute_param.h"

#include "hilog_tag_wrapper.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

bool SkillExecuteParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    skillName_ = Str16ToStr8(parcel.ReadString16());
    auto wantParams = parcel.ReadParcelable<WantParams>();
    if (wantParams == nullptr) {
        return false;
    }
    params_ = std::shared_ptr<WantParams>(wantParams);
    srcEntrance_ = Str16ToStr8(parcel.ReadString16());
    return true;
}

SkillExecuteParam *SkillExecuteParam::Unmarshalling(Parcel &parcel)
{
    auto *param = new (std::nothrow) SkillExecuteParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        return nullptr;
    }
    return param;
}

bool SkillExecuteParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName_));
    parcel.WriteString16(Str8ToStr16(moduleName_));
    parcel.WriteString16(Str8ToStr16(skillName_));
    parcel.WriteParcelable(params_.get());
    parcel.WriteString16(Str8ToStr16(srcEntrance_));
    return true;
}

bool SkillExecuteParam::IsSkillExecute(const AAFwk::Want &want)
{
    return want.HasParameter(SKILL_EXECUTE_PARAM_SKILL_NAME);
}

bool SkillExecuteParam::GenerateFromWant(const AAFwk::Want &want, SkillExecuteParam &param)
{
    const WantParams &wantParams = want.GetParams();
    if (!wantParams.HasParam(SKILL_EXECUTE_PARAM_SKILL_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no skill name in want");
        return false;
    }

    param.bundleName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME);
    param.moduleName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_MODULE_NAME);
    param.skillName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_SKILL_NAME);

    auto params = wantParams.GetWantParams(SKILL_EXECUTE_PARAM_PARAMS);
    param.params_ = std::make_shared<WantParams>(params);
    param.srcEntrance_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRANCE);
    return true;
}

bool SkillExecuteParam::RemoveSkillParam(AAFwk::Want &want)
{
    want.RemoveParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_MODULE_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_SKILL_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_PARAMS);
    want.RemoveParam(SKILL_EXECUTE_PARAM_SRC_ENTRANCE);
    return true;
}

void SkillExecuteParam::WriteToWant(AAFwk::Want &want, const std::string &bundleName,
    const std::string &moduleName, const std::string &skillName,
    const std::shared_ptr<WantParams> &params,
    const std::string &srcEntrance)
{
    want.SetParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME, bundleName);
    want.SetParam(SKILL_EXECUTE_PARAM_MODULE_NAME, moduleName);
    want.SetParam(SKILL_EXECUTE_PARAM_SKILL_NAME, skillName);
    if (params != nullptr) {
        want.SetParam(SKILL_EXECUTE_PARAM_PARAMS, *params);
    }
    if (!srcEntrance.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_SRC_ENTRANCE, srcEntrance);
    }
}

} // namespace AppExecFwk
} // namespace OHOS
