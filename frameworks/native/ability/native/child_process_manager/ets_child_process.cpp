/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_child_process.h"

#include "hilog_tag_wrapper.h"
#include "ani_common_child_process_param.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<ChildProcess> EtsChildProcess::Create(const std::unique_ptr<Runtime>& runtime)
{
    return std::make_shared<EtsChildProcess>(static_cast<STSRuntime &>(*runtime));
}

EtsChildProcess::EtsChildProcess(STSRuntime &etsRuntime) : etsRuntime_(etsRuntime) {}

EtsChildProcess::~EtsChildProcess() {}

bool EtsChildProcess::Init(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "EtsChildProcess::Init Call");
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
    if (srcPath.rfind(".") != std::string::npos) {
        srcPath.erase(srcPath.rfind("."));
    }
    srcPath.append(".abc");
    std::string moduleName(info->moduleName);
    moduleName.append("::").append(info->name);

    etsChildProcessObj_ = etsRuntime_.LoadModule(moduleName, srcPath, info->hapPath, info->isEsModule, false, "");
    if (etsChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null etsChildProcessObj_");
        return false;
    }
    return true;
}

void EtsChildProcess::OnStart()
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "EtsChildProcess OnStart Call");
    ChildProcess::OnStart();
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "env null");
        return;
    }
    ani_ref undefinedRef;
    ani_status status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetUndefined failed status: %{public}d", status);
        return;
    }
    CallObjectMethod(env, "onStart", static_cast<ani_object>(undefinedRef));
}

void EtsChildProcess::OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "EtsChildProcess OnStart args Call");
    if (!args) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null args");
        return;
    }
    ChildProcess::OnStart(args);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "env null");
        return;
    }

    ani_object etsArgs = WrapChildProcessArgs(env, *args);
    CallObjectMethod(env, "onStart", etsArgs);
}

void EtsChildProcess::CallObjectMethod(ani_env* env, const char *name, ani_object etsArgs)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "CallObjectMethod Call, name:%{public}s", name);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "env null");
        return;
    }
    ani_method method {};
    ani_status status = ANI_ERROR;
    if ((status = env->Class_FindMethod(etsChildProcessObj_->aniCls, name,
        "L@ohos/app/ability/ChildProcessArgs/ChildProcessArgs;:V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindMethod status: %{public}d", status);
        return;
    }
    if ((status = env->Object_CallMethod_Void(etsChildProcessObj_->aniObj, method, etsArgs)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_CallMethod_Void status: %{public}d", status);
        return;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS