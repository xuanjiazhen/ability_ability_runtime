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

#ifndef ACE_NAPI_HOOK_NATIVE_REFERENCE_ABI_H
#define ACE_NAPI_HOOK_NATIVE_REFERENCE_ABI_H

// The NativeReference interface below is derived from OpenHarmony arkui_napi
// (OpenHarmony-v5.0.0-Release), licensed under Apache License 2.0.

#include <cstdint>

#include "napi_ohos_compat.h"

class NativeEngine;

// ABI declaration mirrored from OpenHarmony arkui_napi's NativeReference.
// The public SDK exposes napi_ref as an opaque pointer but system components
// cast it back to this internal interface and may own it through smart pointers.
// Keep virtual methods in platform order; changing or removing a slot breaks
// callers compiled against libace_napi.z.so.
class NativeReference {
public:
    virtual ~NativeReference() = default;
    virtual uint32_t Ref() = 0;
    virtual uint32_t Unref() = 0;
    virtual napi_value Get(NativeEngine* engine)
    {
        return Get();
    }
    virtual napi_value Get() = 0;
    virtual void* GetData()
    {
        return nullptr;
    }
    virtual operator napi_value() = 0;
    virtual void SetDeleteSelf() = 0;
    virtual bool GetDeleteSelf() const
    {
        return false;
    }
    virtual uint32_t GetRefCount() = 0;
    virtual bool GetFinalRun() = 0;
    virtual napi_value GetNapiValue() = 0;
    virtual void ResetFinalizer()
    {
    }

    // HarmonyOS 5.0's ArkNativeReference vtable contains this additional slot
    // immediately after ResetFinalizer. Its implementation returns an opaque
    // global-reference slot address; the proxy has no platform slot to expose.
    virtual uintptr_t GetGlobalRefSlotAddress() const
    {
        return 0;
    }
};

#endif

