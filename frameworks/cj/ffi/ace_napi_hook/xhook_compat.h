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

#ifndef ACE_NAPI_HOOK_XHOOK_COMPAT_H
#define ACE_NAPI_HOOK_XHOOK_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int xhook_register(const char* symbol, void* new_func);
int xhook_ignore(const char* symbol);
int xhook_refresh(int async);
int xhook_refresh_library(const char* soname, int async);
int xhook_refresh_libraries(const char* const* sonames, size_t count, int async);
void xhook_clear(void);
void xhook_enable_debug(int flag);

#ifdef __cplusplus
}
#endif

#endif

