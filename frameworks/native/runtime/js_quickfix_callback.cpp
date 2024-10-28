/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_quickfix_callback.h"

#include "file_mapper.h"
#include "file_path_utils.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
    constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
    constexpr char WIDGET_ABC_PATH[] = "/ets/widgets.abc";
}

std::string JsQuickfixCallback::getModuleName(const std::string &baseFileName)
{
    if (baseFileName.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "huasmile baseFileName is empty!");
        return "";
    }

    std::string moduleName;
    if (type_ == AbilityRuntime::RuntimeType::RUNTIME_APP) {
        int baseFileNameLen = static_cast<int>(baseFileName.length());
        int prefixLen = strlen(BUNDLE_INSTALL_PATH);
        int suffixLen = strlen(MERGE_ABC_PATH);
        int moduleLen = baseFileNameLen - prefixLen - suffixLen;
        if (moduleLen < 0) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "huasmile invalid baseFileName!");
            return "";
        }
        moduleName = baseFileName.substr(prefixLen, moduleLen);
    } else if (type_ == AbilityRuntime::RuntimeType::RUNTIME_FORM) {
        std::string fullPath = patchVersion_ + "/";
        size_t patchPos = baseFileName.find(fullPath);
        if (patchPos == std::string::npos) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "huasmile invalid patchPos!");
            return "";
        }
        size_t endPos = baseFileName.find(WIDGET_ABC_PATH);
        if (endPos == std::string::npos) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "huasmile invalid endPos!");
            return "";
        }
        moduleName = baseFileName.substr(patchPos + fullPath.size(), endPos - patchPos - fullPath.size());
        TAG_LOGD(AAFwkTag::JSRUNTIME, "huasmile moduleName:%{public}s", moduleName.c_str());
    } else {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "huasmile not support runtime_type");
        return "";
    }
    return moduleName;
}
bool JsQuickfixCallback::operator()(std::string baseFileName, std::string &patchFileName,
                                    uint8_t **patchBuffer, size_t &patchSize)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "baseFileName: %{private}s", baseFileName.c_str());
    auto position = baseFileName.find(".abc");
    if (position == std::string::npos) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, ".abc not found");
        return false;
    }

    std::string moduleName = getModuleName(baseFileName);
    if (moduleName.empty()) {
        return false;
    }

    auto it = moduleAndHqfPath_.find(moduleName);
    if (it == moduleAndHqfPath_.end()) {
        return false;
    }

    std::string resolvedHqfFile;
    std::string hqfFile = it->second;
    if (type_ == AbilityRuntime::RuntimeType::RUNTIME_APP) {
        resolvedHqfFile = AbilityBase::GetLoadPath(hqfFile);
    } else if (type_ == AbilityRuntime::RuntimeType::RUNTIME_FORM) {
        resolvedHqfFile = hqfFile;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "hqfFile: %{private}s, resolvedHqfFile: %{private}s", hqfFile.c_str(),
        resolvedHqfFile.c_str());
    auto data = JsRuntime::GetSafeData(resolvedHqfFile, patchFileName);
    if (data == nullptr) {
        if (patchFileName.empty()) {
            TAG_LOGI(AAFwkTag::JSRUNTIME, "No need to load patch cause no ets. path: %{private}s",
                resolvedHqfFile.c_str());
            return true;
        }
        return false;
    }
    *patchBuffer = data->GetDataPtr();
    TAG_LOGD(AAFwkTag::JSRUNTIME, "patchFileName: %{private}s", patchFileName.c_str());
    patchSize = data->GetDataLen();
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
