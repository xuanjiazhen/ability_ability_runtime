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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_BMS_QUICK_FIX_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_MOCK_BMS_QUICK_FIX_MANAGER_CLIENT_H

#include "gmock/gmock.h"

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {

class QuickFixManagerClient {
public:
    static QuickFixManagerClient &GetInstance();

    int32_t DeployQuickFix(const std::vector<std::string> &bundleFilePaths,
        const sptr<IQuickFixStatusCallback> &statusCallback, bool isDebug = false,
        const std::string &inputTargetPath = "", bool isReplace = false);

    int32_t SwitchQuickFix(const std::string &bundleName, bool enable,
        const sptr<IQuickFixStatusCallback> &statusCallback);

    int32_t DeleteQuickFix(const std::string &bundleName,
        const sptr<IQuickFixStatusCallback> &statusCallback);

    int32_t CreateFd(const std::string &fileName, int32_t &fd, std::string &path);
    int32_t CopyFiles(const std::vector<std::string> &sourceFiles, std::vector<std::string> &destFiles);

private:
    QuickFixManagerClient();
    ~QuickFixManagerClient();
    DISALLOW_COPY_AND_MOVE(QuickFixManagerClient);
};
}
}
#endif  // OHOS_ABILITY_RUNTIME_MOCK_BMS_QUICK_FIX_MANAGER_CLIENT_H