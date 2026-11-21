/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_APP_SCHEDULER_STUB_PROBE_H
#define OHOS_ABILITY_RUNTIME_APP_SCHEDULER_STUB_PROBE_H

#include <cstdint>

namespace OHOS {
namespace AAFwk {
/*
 * Probe into the link-time AppScheduler stub used by this UT binary.
 *
 * mock/include/app_scheduler.h is a byte-identical copy of the real header so
 * that the AppScheduler class has one single definition process-wide (the
 * DelayedSingleton<AppScheduler> static instance is shared between this
 * executable and libabilityms.z.so through ELF symbol interposition; a class
 * with a different layout would make the two sides disagree on member
 * offsets). mock/src/mock_app_scheduler.cpp therefore only overrides the
 * attribution channel whose behaviour the dedup regression tests need to
 * observe; every other AppScheduler symbol resolves to the real
 * implementation in libabilityms.z.so.
 *
 * The probe state itself is deliberately kept OUT of the AppScheduler class
 * (free functions plus file-static storage) so the copied class definition
 * stays byte-identical to the real one.
 */
struct AppSchedulerStubState {
    // false: registration fails like an unreachable appmgr service (default);
    // true: registration succeeds and hands out incrementing handles.
    bool attributeSucceeds = false;
    // number of registration attempts seen by the stub.
    int32_t attributeCalls = 0;
    // number of unregister attempts seen by the stub.
    int32_t unregisterCalls = 0;
    // first handle handed out when attributeSucceeds is true.
    int64_t nextAttributionId = 4701;
};

AppSchedulerStubState &AppSchedulerStub();
void AppSchedulerStubReset();
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_SCHEDULER_STUB_PROBE_H
