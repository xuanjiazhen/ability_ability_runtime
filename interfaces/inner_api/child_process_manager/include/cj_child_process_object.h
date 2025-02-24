/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_CHILD_PROCESS_OBJECT_H
#define OHOS_ABILITY_RUNTIME_CJ_CHILD_PROCESS_OBJECT_H
#include <cstdint>
#include <memory>

#include "child_process_args.h"

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif


struct CArrInt32 {
    int32_t* head;
    int64_t size;
};

struct CArrString {
    char** head;
    int64_t size;
};

struct CArrStrInt32 {
    CArrString keys;
    CArrInt32 values;
};


namespace OHOS {
namespace AbilityRuntime {

class CJChildProcessObject {
public:
    explicit CJChildProcessObject(int64_t id) : id_(id) {}
    ~CJChildProcessObject();
    static std::shared_ptr<CJChildProcessObject> Create(const std::string& index);
    void OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args = nullptr);

private:
    int64_t id_ = 0;
    CArrStrInt32 toCMap(const std::map<std::string, int32_t>& fds);
};
} // namespace AbilityRuntime
} // namespace OHOS

extern "C" {
struct CJChildProcessFuncs {
    int64_t (*cjChildProcessCreate)(const char* index);
    void (*cjChildProcessOnStart)(int64_t id);
    void (*cjChildProcessOnStartWithArgs)(int64_t id, const char* entryParams, CArrStrInt32 fds);
    void (*cjChildProcessRelease)(int64_t id);
};

CJ_EXPORT void RegisterCJChildProcessFuncs(void (*registerFunc)(CJChildProcessFuncs*));
};
#endif