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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_CLIENT_EXPORT_H
#define OHOS_ABILITY_RUNTIME_INTENT_CLIENT_EXPORT_H

// 配合 BUILD.gn 的 -fvisibility=hidden 使用:
// - 编译期默认所有符号 hidden
// - 显式标 INTENT_CLIENT_API 的类/函数才导出, vtable / typeinfo 跟随类 visibility
// - 配合 libintent_client.map 进一步收紧最终 .dynsym 表
// 注意: 不开 -fvisibility-inlines-hidden, 避免 inline 函数 vtable 跨 .so 漂移

#if defined(__GNUC__) || defined(__clang__)
#define INTENT_CLIENT_API __attribute__((visibility("default")))
#else
#define INTENT_CLIENT_API
#endif

#endif  // OHOS_ABILITY_RUNTIME_INTENT_CLIENT_EXPORT_H
