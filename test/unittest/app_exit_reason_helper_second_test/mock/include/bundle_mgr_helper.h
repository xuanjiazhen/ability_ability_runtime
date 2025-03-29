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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include "ability_keep_alive_service.h"
#include "app_control_interface.h"
#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    BundleMgrHelper();

    ~BundleMgrHelper();

    ErrCode GetNameAndIndexForUid(const int32_t uid, std::string& bundleName, int32_t& appIndex);

    sptr<IAppControlMgr> GetAppControlProxy()
    {
        return nullptr;
    }

    bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo& appInfo)
    {
        return false;
    }

    ErrCode GetNameForUid(const int32_t uid, std::string& name)
    {
        return 0;
    }

    bool QueryAppGalleryBundleName(std::string& bundleName)
    {
        return false;
    }

public:
    static bool getNameAndIndexForUid_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H