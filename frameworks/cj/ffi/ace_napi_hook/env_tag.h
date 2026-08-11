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

#ifndef ACE_NAPI_HOOK_ENV_TAG_H
#define ACE_NAPI_HOOK_ENV_TAG_H

#include <cstdint>
#include "napi_ohos_compat.h"

namespace ace_napi_hook {

struct mixed_napi_env {
  napi_env tagged_env;

  enum tag_t {
    ArktsDyn = 0,
    ArktsSta = 1,
    Cangjie = 2,
  };

  static constexpr int TAG_BIT_LOW = 56;
  static constexpr int TAG_BIT_HIGH = 63;
  static constexpr uint64_t TAG_BIT_MASK = 0xFFull << TAG_BIT_LOW;

  static inline tag_t get_tag(napi_env env) {
    return static_cast<tag_t>(
        (reinterpret_cast<uintptr_t>(env) & TAG_BIT_MASK) >> TAG_BIT_LOW);
  }

  static inline napi_env wrap(napi_env env, tag_t tag) {
    return reinterpret_cast<napi_env>(
        reinterpret_cast<uintptr_t>(env) |
        (static_cast<uintptr_t>(tag) << TAG_BIT_LOW));
  }

  static inline napi_env unwrap(napi_env env) {
    return reinterpret_cast<napi_env>(reinterpret_cast<uintptr_t>(env) &
                                      ~TAG_BIT_MASK);
  }

};

template <typename T>
inline bool is_tagged_cangjie_pointer(T ptr) {
  const auto raw = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(ptr));
  const auto tag = (raw & mixed_napi_env::TAG_BIT_MASK) >> mixed_napi_env::TAG_BIT_LOW;
  return tag == static_cast<uint64_t>(mixed_napi_env::Cangjie);
}

}  // namespace ace_napi_hook

#endif

