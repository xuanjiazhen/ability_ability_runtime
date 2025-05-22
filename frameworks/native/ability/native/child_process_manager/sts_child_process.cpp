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

#include "sts_child_process.h"

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<ChildProcess> StsChildProcess::Create(const std::unique_ptr<Runtime> &runtime)
{
    return std::make_shared<StsChildProcess>(static_cast<STSRuntime&>(*runtime));
}

StsChildProcess::StsChildProcess(STSRuntime &stsRuntime) : stsRuntime_(stsRuntime) {}

StsChildProcess::~StsChildProcess()
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
}

void StsChildProcess::ResetEnv(ani_env* env)
{
    if (env) {
        env->DescribeError();
        env->ResetError();
    }
}

bool StsChildProcess::Init(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "Init called");
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

    stsChildProcessObj_ = stsRuntime_.LoadModule(moduleName, srcPath, info->hapPath, info->isEsModule, false, "");
    if (stsChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null stsChildProcessObj_");
        return false;
    }
    return true;
}

void StsChildProcess::OnStart()
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "OnStart called");
    ChildProcess::OnStart();
    CallObjectMethod(false, "onStart", ":V");
}

void StsChildProcess::OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "StsChildProcess OnStart called");
    if (!args) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null args");
        return;
    }
    ChildProcess::OnStart(args);

    CallObjectMethod(false, "onStart", "L@ohos/app/ability/ChildProcessArgs/ChildProcessArgs;:V");
}

bool StsChildProcess::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    if (stsChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "stsChildProcessObj_ nullptr");
        return false;
    }

    auto env = stsRuntime_.GetAniEnv();
    ResetEnv(env);
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(stsChildProcessObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = 0;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(stsChildProcessObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "status: %{public}d", status);
            stsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(stsChildProcessObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "status: %{public}d", status);
        stsRuntime_.HandleUncaughtError();
        return false;
    }
    va_end(args);
    return false;
}
}
}
