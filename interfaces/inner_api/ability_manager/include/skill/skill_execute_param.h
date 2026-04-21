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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_PARAM_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_PARAM_H

#include <string>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {

using WantParams = OHOS::AAFwk::WantParams;

// Want parameter keys for skill execution
constexpr char SKILL_EXECUTE_PARAM_BUNDLE_NAME[] = "ohos.skill.executeParam.bundleName";
constexpr char SKILL_EXECUTE_PARAM_MODULE_NAME[] = "ohos.skill.executeParam.moduleName";
constexpr char SKILL_EXECUTE_PARAM_SKILL_NAME[] = "ohos.skill.executeParam.skillName";
constexpr char SKILL_EXECUTE_PARAM_PARAMS[] = "ohos.skill.executeParam.params";

class SkillExecuteParam : public Parcelable {
public:
    SkillExecuteParam() = default;
    ~SkillExecuteParam() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static SkillExecuteParam *Unmarshalling(Parcel &parcel);

    static bool IsSkillExecute(const AAFwk::Want &want);
    static bool GenerateFromWant(const AAFwk::Want &want, SkillExecuteParam &param);
    static bool RemoveSkillParam(AAFwk::Want &want);
    static void WriteToWant(AAFwk::Want &want, const std::string &bundleName,
        const std::string &moduleName, const std::string &skillName,
        const std::shared_ptr<WantParams> &params);

    std::string bundleName_;
    std::string moduleName_;
    std::string skillName_;
    std::shared_ptr<WantParams> params_;
};

} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_PARAM_H
