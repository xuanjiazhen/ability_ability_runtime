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
#ifndef FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_COMMON_H
#define FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_COMMON_H

#define DECLARE_NAPI_PROPERTY(name, val)                                       \
    {                                                                          \
        (name), nullptr, nullptr, nullptr, nullptr, val, napi_default, nullptr \
    }

#define DECLARE_NAPI_STATIC_PROPERTY(name, val)                               \
    {                                                                         \
        (name), nullptr, nullptr, nullptr, nullptr, val, napi_static, nullptr \
    }

#define DECLARE_NAPI_FUNCTION(name, func)                                                            \
    {                                                                             \
        (name), nullptr, (func), nullptr, nullptr, nullptr, napi_default, nullptr \
    }

#define DECLARE_NAPI_FUNCTION_WITH_DATA(name, func, data)                         \
    {                                                                             \
        (name), nullptr, (func), nullptr, nullptr, nullptr, napi_default, data    \
    }

#define DECLARE_NAPI_STATIC_FUNCTION(name, func)                                 \
    {                                                                            \
        (name), nullptr, (func), nullptr, nullptr, nullptr, napi_static, nullptr \
    }

#define DECLARE_NAPI_GETTER(name, getter)                                           \
    {                                                                               \
        (name), nullptr, nullptr, (getter), nullptr, nullptr, napi_default, nullptr \
    }

#define DECLARE_NAPI_SETTER(name, setter)                                           \
    {                                                                               \
        (name), nullptr, nullptr, nullptr, (setter), nullptr, napi_default, nullptr \
    }

#define DECLARE_NAPI_GETTER_SETTER(name, getter, setter)                             \
    {                                                                                \
        (name), nullptr, nullptr, (getter), (setter), nullptr, napi_default, nullptr \
    }

#define DECLARE_KEYCODE_EVENT(name)                                                 \
static napi_value GetKeyCodeEvent##name(napi_env env, napi_callback_info info)      \
{                                                                                   \
    napi_value retValue = 0;                                                        \
    if (g_key##name##IsTrue) {                                                      \
        napi_create_int32(env, g_key##name##IsTrue, &retValue);                     \
    }

#endif /* FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_COMMON_H */
