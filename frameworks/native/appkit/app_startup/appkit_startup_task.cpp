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

#include <cstdint>

namespace {
// ABC bytecode of framework-internal startup task modules, embedded in this library at build time.
// These modules are not registered as napi modules and this library is not installed in napi
// module search paths, so the bytecode is not reachable via requireNapi by applications.
constexpr int32_t INVALID_ABC_LEN = -1;

int32_t GetAbcLen(const char* start, const char* end)
{
    if (start == nullptr || end == nullptr || end < start) {
        return INVALID_ABC_LEN;
    }
    return static_cast<int32_t>(end - start);
}
}

extern const char _binary_async_task_excutor_abc_start[];
extern const char _binary_async_task_excutor_abc_end[];
extern const char _binary_async_task_callback_abc_start[];
extern const char _binary_async_task_callback_abc_end[];

extern "C" __attribute__((visibility("default"))) const uint8_t* GetAsyncTaskExcutorAbc(int32_t* len)
{
    if (len != nullptr) {
        *len = GetAbcLen(_binary_async_task_excutor_abc_start, _binary_async_task_excutor_abc_end);
    }
    return reinterpret_cast<const uint8_t*>(_binary_async_task_excutor_abc_start);
}

extern "C" __attribute__((visibility("default"))) const uint8_t* GetAsyncTaskCallbackAbc(int32_t* len)
{
    if (len != nullptr) {
        *len = GetAbcLen(_binary_async_task_callback_abc_start, _binary_async_task_callback_abc_end);
    }
    return reinterpret_cast<const uint8_t*>(_binary_async_task_callback_abc_start);
}
