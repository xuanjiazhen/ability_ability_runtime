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
    arkTSPath_ = Str16ToStr8(parcel.ReadString16());
    funcName_ = Str16ToStr8(parcel.ReadString16());
    int32_t argc = parcel.ReadInt32();
    for (int32_t i = 0; i < argc; i++) {
        argv_.push_back(Str16ToStr8(parcel.ReadString16()));
    }
    int32_t srcCount = parcel.ReadInt32();
    for (int32_t i = 0; i < srcCount; i++) {
        srcEntries_.push_back(Str16ToStr8(parcel.ReadString16()));
    }
    requestCode_ = parcel.ReadInt64();
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
    parcel.WriteString16(Str8ToStr16(arkTSPath_));
    parcel.WriteString16(Str8ToStr16(funcName_));
    parcel.WriteInt32(static_cast<int32_t>(argv_.size()));
    for (const auto &arg : argv_) {
        parcel.WriteString16(Str8ToStr16(arg));
    }
    parcel.WriteInt32(static_cast<int32_t>(srcEntries_.size()));
    for (const auto &entry : srcEntries_) {
        parcel.WriteString16(Str8ToStr16(entry));
    }
    parcel.WriteInt64(requestCode_);
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
    param.arkTSPath_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_ARKTS_PATH);
    param.funcName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_FUNC_NAME);
    int32_t argc = 0;
    auto countStr = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_ARGV_COUNT);
    if (!countStr.empty()) {
        argc = std::stoi(countStr);
    }
    for (int32_t i = 0; i < argc; i++) {
        auto key = std::string(SKILL_EXECUTE_PARAM_ARGV_PREFIX) + std::to_string(i);
        param.argv_.push_back(wantParams.GetStringParam(key));
    }
    int32_t srcCount = 0;
    auto srcCountStr = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT);
    if (!srcCountStr.empty()) {
        srcCount = std::stoi(srcCountStr);
    }
    for (int32_t i = 0; i < srcCount; i++) {
        auto key = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + std::to_string(i);
        param.srcEntries_.push_back(wantParams.GetStringParam(key));
    }
    auto requestCodeStr = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_REQUEST_CODE);
    if (!requestCodeStr.empty()) {
        param.requestCode_ = std::stoll(requestCodeStr);
    }
    return true;
}

bool SkillExecuteParam::RemoveSkillParam(AAFwk::Want &want)
{
    want.RemoveParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_MODULE_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_SKILL_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_ARKTS_PATH);
    want.RemoveParam(SKILL_EXECUTE_PARAM_FUNC_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_ARGV_COUNT);
    want.RemoveParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT);
    want.RemoveParam(SKILL_EXECUTE_PARAM_REQUEST_CODE);
    auto params = want.GetParams();
    auto countStr = params.GetStringParam(SKILL_EXECUTE_PARAM_ARGV_COUNT);
    if (!countStr.empty()) {
        int32_t argc = std::stoi(countStr);
        for (int32_t i = 0; i < argc; i++) {
            auto key = std::string(SKILL_EXECUTE_PARAM_ARGV_PREFIX) + std::to_string(i);
            want.RemoveParam(key);
        }
    }
    auto srcCountStr = params.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT);
    if (!srcCountStr.empty()) {
        int32_t srcCount = std::stoi(srcCountStr);
        for (int32_t i = 0; i < srcCount; i++) {
            auto key = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + std::to_string(i);
            want.RemoveParam(key);
        }
    }
    return true;
}

void SkillExecuteParam::WriteToWant(AAFwk::Want &want, const std::string &bundleName,
    const std::string &moduleName, const std::string &skillName,
    const std::string &arkTSPath, const std::string &funcName,
    const std::vector<std::string> &argv,
    const std::vector<std::string> &srcEntries,
    int64_t requestCode)
{
    want.SetParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME, bundleName);
    want.SetParam(SKILL_EXECUTE_PARAM_MODULE_NAME, moduleName);
    want.SetParam(SKILL_EXECUTE_PARAM_SKILL_NAME, skillName);
    if (!arkTSPath.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_ARKTS_PATH, arkTSPath);
    }
    if (!funcName.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_FUNC_NAME, funcName);
    }
    if (!argv.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_ARGV_COUNT, std::to_string(argv.size()));
        for (size_t i = 0; i < argv.size(); i++) {
            auto key = std::string(SKILL_EXECUTE_PARAM_ARGV_PREFIX) + std::to_string(i);
            want.SetParam(key, argv[i]);
        }
    }
    if (!srcEntries.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT, std::to_string(srcEntries.size()));
        for (size_t i = 0; i < srcEntries.size(); i++) {
            auto key = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + std::to_string(i);
            want.SetParam(key, srcEntries[i]);
        }
    }
    if (requestCode != 0) {
        want.SetParam(SKILL_EXECUTE_PARAM_REQUEST_CODE, std::to_string(requestCode));
    }
}

} // namespace AppExecFwk
} // namespace OHOS
