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

#ifndef OHOS_ABILITY_RUNTIME_WANT_FD_GUARD_H
#define OHOS_ABILITY_RUNTIME_WANT_FD_GUARD_H

#include "want.h"

namespace OHOS {
namespace AAFwk {

/**
 * @brief RAII guard that adopts all LEGACY fd states in a Want on construction.
 *
 * @note NOT YET WIRED (0 callers as of this PR). Target call sites (P2b/S1):
 *       - ui_ability_lifecycle_manager receive paths (SCB)
 *       - mission_info.cpp read-from-parcel
 *       - pending_want_key receive path
 *       Usage intent: construct at IPC receive boundary after Want deserialization;
 *       destructor is a no-op (reference release drives RAII close via ~WantFdState).
 *       Performance note: RECURSIVE mode traverses nested params + fstat per fd.
 *
 * @deprecated-in-future: this class retires when CloseAllFd/RemoveAllFd production
 *       calls reach zero and receive-boundary adopt becomes the sole path (T-2/T-4).
 */
class WantFdGuard {
public:
    explicit WantFdGuard(Want &want, FdTraversalMode mode = FdTraversalMode::RECURSIVE) : want_(want)
    {
        (void)want_.AdoptAllLegacyFd(mode);
    }

    ~WantFdGuard() = default;

    WantFdGuard(const WantFdGuard &) = delete;
    WantFdGuard &operator=(const WantFdGuard &) = delete;

private:
    Want &want_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_WANT_FD_GUARD_H
