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

#include "keepaliveprocessmanagerfourth_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ability_keep_alive_service.h"
#include "keep_alive_process_manager.h"
#undef private
#include "ability_fuzz_util.h"
#include "ability_util.h"
#include "app_mgr_client.h"
#include "parameters.h"
#include "permission_verification.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string bundleName = fdp.ConsumeRandomLengthString(64);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    BundleInfo bundleInfo;
    AbilityFuzzUtil::GetRandomBundleInfo(fdp, bundleInfo);
    std::vector<BundleInfo> bundleInfos = {bundleInfo};

    auto &mgr = KeepAliveProcessManager::GetInstance();
    mgr.SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    (void)mgr.KeepAliveIsRestartAfterUpdate(uid);
    (void)mgr.KeepAliveCheckNeedRestartAfterUpgrade(uid);
    mgr.StartKeepAliveAfterAppUpgrade(bundleInfos, uid);

    // Fuzz the CheckStatusBarTask helper class.
    CheckStatusBarTask statusBarTask(uid, []() {});
    (void)statusBarTask.GetUid();
    statusBarTask.Cancel();
    statusBarTask.Run();

    CheckStatusBarTask statusBarTask2(uid, []() {});
    statusBarTask2.Run();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}