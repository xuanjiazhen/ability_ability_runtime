/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "cj_child_process_impl.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<ChildProcess> CJChildProcessImpl::Create(const std::unique_ptr<Runtime>& runtime)
{
    return std::make_shared<CJChildProcessImpl>(static_cast<CJRuntime&>(*runtime));
}

CJChildProcessImpl::CJChildProcessImpl(CJRuntime& cjRuntime) : cjRuntime_(cjRuntime) {}

bool CJChildProcessImpl::Init(const std::shared_ptr<ChildProcessStartInfo>& info)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null info");
        return false;
    }
    bool ret = ChildProcess::Init(info);
    if (!ret) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "ChildProcess init failed");
        return false;
    }
    if (info->srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Empty info srcEntry");
        return false;
    }
    std::string srcPath = info->srcEntry;
    if (srcPath.find("/") != std::string::npos) {
        srcPath = srcPath.substr(srcPath.find("/") + 1);
    }

    bool isLoaded = cjRuntime_.IsAppLibLoaded();
    if (isLoaded) {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "AppLib is Loaded");
    }

    cjChildProcessObj_ = CJChildProcessObject::Create(srcPath);
    if (cjChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null cjChildProcessObj_");
        return false;
    }
    return true;
}

void CJChildProcessImpl::OnStart()
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    ChildProcess::OnStart();
    cjChildProcessObj_->OnStart();
}

void CJChildProcessImpl::OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "CJChildProcessImpl OnStart called");
    if (!args) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null args");
        return;
    }
    ChildProcess::OnStart(args);
    cjChildProcessObj_->OnStart(args);
}
} // namespace AbilityRuntime
} // namespace OHOS