/*
 * Author: Keyang Li <2024865311@qq.com>
 * Affiliation: Shandong University
 * Mentor: Hongjun Dai <dahogn@sdu.edu.cn>
 * Mentor: Hongbo Chen <hongbochen@sdu.edu.cn>
 */

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

#define LOG_TAG "NativeUriLazy"

#include <cstdlib>
#include <dlfcn.h>
#include <utility>

#include "datashare_log.h"
#include "uri.h"

namespace OHOS::DataShare {
namespace {
void *ResolveNativeUri(const char *name) {
  // Retain the original provider for live objects, vtables and callbacks.
  static void *handle = [] {
    void *result = dlopen("libzuri.z.so", RTLD_NOW | RTLD_LOCAL);
    if (result == nullptr) {
      LOG_ERROR("native URI provider load failed");
      std::abort();
    }
    return result;
  }();
  void *entry = dlsym(handle, name);
  if (entry == nullptr) {
    LOG_ERROR("native URI symbol resolve failed");
    std::abort();
  }
  return entry;
}
} // namespace

__attribute__((visibility("hidden"))) void
NativeUri0(Uri *self, Uri &&other) asm("__wrap__ZN4OHOS3UriC1EOS0_");
void NativeUri0(Uri *self, Uri &&other) {
  using Function = void (*)(Uri *, Uri &&);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriC1EOS0_"));
  return entry(self, std::move(other));
}

__attribute__((visibility("hidden"))) void
NativeUri1(Uri *self, const std::string &text) asm(
    "__wrap__ZN4OHOS3UriC1ERKNSt3__h12basic_stringIcNS1_11char_traitsIcEENS1_"
    "9allocatorIcEEEE");
void NativeUri1(Uri *self, const std::string &text) {
  using Function = void (*)(Uri *, const std::string &);
  static Function entry = reinterpret_cast<Function>(
      ResolveNativeUri("_ZN4OHOS3UriC1ERKNSt3__h12basic_stringIcNS1_11char_"
                       "traitsIcEENS1_9allocatorIcEEEE"));
  return entry(self, text);
}

__attribute__((visibility("hidden"))) void
NativeUri2(Uri *self, const Uri &other) asm("__wrap__ZN4OHOS3UriC1ERKS0_");
void NativeUri2(Uri *self, const Uri &other) {
  using Function = void (*)(Uri *, const Uri &);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriC1ERKS0_"));
  return entry(self, other);
}

__attribute__((visibility("hidden"))) void
NativeUri3(Uri *self) asm("__wrap__ZN4OHOS3UriD1Ev");
void NativeUri3(Uri *self) {
  using Function = void (*)(Uri *);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriD1Ev"));
  return entry(self);
}

__attribute__((visibility("hidden"))) std::string
NativeUri4(const Uri *self) asm("__wrap__ZNK4OHOS3Uri8ToStringEv");
std::string NativeUri4(const Uri *self) {
  using Function = std::string (*)(const Uri *);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZNK4OHOS3Uri8ToStringEv"));
  return entry(self);
}

__attribute__((visibility("hidden"))) bool
NativeUri5(const Uri *self,
           const Uri &other) asm("__wrap__ZNK4OHOS3UrieqERKS0_");
bool NativeUri5(const Uri *self, const Uri &other) {
  using Function = bool (*)(const Uri *, const Uri &);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZNK4OHOS3UrieqERKS0_"));
  return entry(self, other);
}

} // namespace OHOS::DataShare
