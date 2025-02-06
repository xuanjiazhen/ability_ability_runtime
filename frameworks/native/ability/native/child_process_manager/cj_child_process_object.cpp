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

#include "cj_child_process_object.h"

#include <string>

#include "cj_utils_ffi.h"
#include "hilog_tag_wrapper.h"

namespace {
CJChildProcessFuncs g_cjChildProcessFuncs = {};
}

void RegisterCJChildProcessFuncs(void (*registerFunc)(CJChildProcessFuncs*))
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "RegisterCJChildProcessFuncs called");
    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null registerFunc");
        return;
    }
    registerFunc(&g_cjChildProcessFuncs);
    TAG_LOGD(AAFwkTag::PROCESSMGR, "RegisterCJChildProcessFuncs end");
}

namespace OHOS {
namespace AbilityRuntime {
CJChildProcessObject::~CJChildProcessObject()
{
    if (g_cjChildProcessFuncs.cjChildProcessRelease) {
        g_cjChildProcessFuncs.cjChildProcessRelease(id_);
    }
    id_ = 0;
}

std::shared_ptr<CJChildProcessObject> CJChildProcessObject::Create(const std::string& index)
{
    if (!g_cjChildProcessFuncs.cjChildProcessCreate) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null cjChildProcessCreate");
        return nullptr;
    }
    int64_t id = g_cjChildProcessFuncs.cjChildProcessCreate(index.c_str());
    return std::make_shared<CJChildProcessObject>(id);
}

CArrStrInt32 CJChildProcessObject::toCMap(const std::map<std::string, int32_t>& fdsMap)
{
    CArrStrInt32 fds = { .keys = { .head = nullptr, .size = 0 }, .values = { .head = nullptr, .size = 0 } };
    char** keysHead = static_cast<char**>(malloc(sizeof(char*) * fdsMap.size()));
    if (keysHead == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "malloc keysHead failed");
        return fds;
    }

    int32_t* valuesHead = static_cast<int32_t*>(malloc(sizeof(int32_t) * fdsMap.size()));
    if (valuesHead == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "malloc valuesHead failed");
        free(keysHead);
        return fds;
    }

    int64_t i = 0;
    for (auto& fd : fdsMap) {
        keysHead[i] = CreateCStringFromString(fd.first);
        valuesHead[i] = fd.second;
        i++;
    }
    fds.keys.head = keysHead;
    fds.keys.size = fdsMap.size();
    fds.values.head = valuesHead;
    fds.values.size = fdsMap.size();
    return fds;
}

void CJChildProcessObject::OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "called");
    if (!args) {
        if (!g_cjChildProcessFuncs.cjChildProcessOnStart) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "null cjChildProcessOnStart");
            return;
        }
        g_cjChildProcessFuncs.cjChildProcessOnStart(id_);
    } else {
        if (!g_cjChildProcessFuncs.cjChildProcessOnStartWithArgs) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "null cjChildProcessOnStartWithArgs");
            return;
        }
        auto cFds = toCMap(args->fds);
        auto cEntryParams = CreateCStringFromString(args->entryParams);
        g_cjChildProcessFuncs.cjChildProcessOnStartWithArgs(id_, cEntryParams, cFds);
        return;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS