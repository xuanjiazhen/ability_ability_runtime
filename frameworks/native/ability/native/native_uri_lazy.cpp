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

#include <cstdlib>
#include <dlfcn.h>
#include <utility>

#include "hilog_tag_wrapper.h"
#include "uri.h"

namespace OHOS::AAFwk {
namespace {
void *ResolveNativeUri(const char *name) {
  // Keep the original provider alive for objects, vtables and callbacks.
  static void *handle = [] {
    void *result = dlopen("libzuri.z.so", RTLD_NOW | RTLD_LOCAL);
    if (result == nullptr) {
      TAG_LOGE(AAFwkTag::DEFAULT, "native URI provider load failed");
      std::abort();
    }
    return result;
  }();
  void *entry = dlsym(handle, name);
  if (entry == nullptr) {
    TAG_LOGE(AAFwkTag::DEFAULT, "native URI symbol resolve failed");
    std::abort();
  }
  return entry;
}
} // namespace

__attribute__((visibility("hidden"))) Uri *NativeUri0(Parcel &parcel) asm(
    "__wrap__ZN4OHOS3Uri13UnmarshallingERNS_6ParcelE");
Uri *NativeUri0(Parcel &parcel) {
  using Function = Uri *(*)(Parcel &);
  static Function entry = reinterpret_cast<Function>(
      ResolveNativeUri("_ZN4OHOS3Uri13UnmarshallingERNS_6ParcelE"));
  return entry(parcel);
}

__attribute__((visibility("hidden"))) void
NativeUri1(Uri *self, std::vector<std::string> &parts) asm(
    "__wrap__ZN4OHOS3Uri15GetPathSegmentsERNSt3__h6vectorINS1_12basic_"
    "stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS6_IS8_EEEE");
void NativeUri1(Uri *self, std::vector<std::string> &parts) {
  using Function = void (*)(Uri *, std::vector<std::string> &);
  static Function entry = reinterpret_cast<Function>(ResolveNativeUri(
      "_ZN4OHOS3Uri15GetPathSegmentsERNSt3__h6vectorINS1_12basic_stringIcNS1_"
      "11char_traitsIcEENS1_9allocatorIcEEEENS6_IS8_EEEE"));
  return entry(self, parts);
}

__attribute__((visibility("hidden"))) void
NativeUri2(Uri *self) asm("__wrap__ZN4OHOS3Uri22SetUriWithOriginStringEv");
void NativeUri2(Uri *self) {
  using Function = void (*)(Uri *);
  static Function entry = reinterpret_cast<Function>(
      ResolveNativeUri("_ZN4OHOS3Uri22SetUriWithOriginStringEv"));
  return entry(self);
}

__attribute__((visibility("hidden"))) std::string
NativeUri3(Uri *self) asm("__wrap__ZN4OHOS3Uri7GetHostEv");
std::string NativeUri3(Uri *self) {
  using Function = std::string (*)(Uri *);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3Uri7GetHostEv"));
  return entry(self);
}

__attribute__((visibility("hidden"))) std::string
NativeUri4(Uri *self) asm("__wrap__ZN4OHOS3Uri7GetPathEv");
std::string NativeUri4(Uri *self) {
  using Function = std::string (*)(Uri *);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3Uri7GetPathEv"));
  return entry(self);
}

__attribute__((visibility("hidden"))) std::string
NativeUri5(Uri *self) asm("__wrap__ZN4OHOS3Uri9GetSchemeEv");
std::string NativeUri5(Uri *self) {
  using Function = std::string (*)(Uri *);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3Uri9GetSchemeEv"));
  return entry(self);
}

__attribute__((visibility("hidden"))) void
NativeUri6(Uri *self, Uri &&other) asm("__wrap__ZN4OHOS3UriC1EOS0_");
void NativeUri6(Uri *self, Uri &&other) {
  using Function = void (*)(Uri *, Uri &&);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriC1EOS0_"));
  return entry(self, std::move(other));
}

__attribute__((visibility("hidden"))) void
NativeUri7(Uri *self, const std::string &text) asm(
    "__wrap__ZN4OHOS3UriC1ERKNSt3__h12basic_stringIcNS1_11char_traitsIcEENS1_"
    "9allocatorIcEEEE");
void NativeUri7(Uri *self, const std::string &text) {
  using Function = void (*)(Uri *, const std::string &);
  static Function entry = reinterpret_cast<Function>(
      ResolveNativeUri("_ZN4OHOS3UriC1ERKNSt3__h12basic_stringIcNS1_11char_"
                       "traitsIcEENS1_9allocatorIcEEEE"));
  return entry(self, text);
}

__attribute__((visibility("hidden"))) void
NativeUri8(Uri *self, const Uri &other) asm("__wrap__ZN4OHOS3UriC1ERKS0_");
void NativeUri8(Uri *self, const Uri &other) {
  using Function = void (*)(Uri *, const Uri &);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriC1ERKS0_"));
  return entry(self, other);
}

__attribute__((visibility("hidden"))) void
NativeUri9(Uri *self) asm("__wrap__ZN4OHOS3UriD1Ev");
void NativeUri9(Uri *self) {
  using Function = void (*)(Uri *);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriD1Ev"));
  return entry(self);
}

__attribute__((visibility("hidden"))) Uri &
NativeUri10(Uri *self, Uri &&other) asm("__wrap__ZN4OHOS3UriaSEOS0_");
Uri &NativeUri10(Uri *self, Uri &&other) {
  using Function = Uri &(*)(Uri *, Uri &&);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriaSEOS0_"));
  return entry(self, std::move(other));
}

__attribute__((visibility("hidden"))) Uri &
NativeUri11(Uri *self, const Uri &other) asm("__wrap__ZN4OHOS3UriaSERKS0_");
Uri &NativeUri11(Uri *self, const Uri &other) {
  using Function = Uri &(*)(Uri *, const Uri &);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZN4OHOS3UriaSERKS0_"));
  return entry(self, other);
}

__attribute__((visibility("hidden"))) std::string
NativeUri12(const Uri *self) asm("__wrap__ZNK4OHOS3Uri8ToStringEv");
std::string NativeUri12(const Uri *self) {
  using Function = std::string (*)(const Uri *);
  static Function entry =
      reinterpret_cast<Function>(ResolveNativeUri("_ZNK4OHOS3Uri8ToStringEv"));
  return entry(self);
}

} // namespace OHOS::AAFwk
