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

#ifndef IABILITY_MANAGER_MOCK_H
#define IABILITY_MANAGER_MOCK_H

#include "iremote_object.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityManagerMock : public IBundleMgr {
public:
    AbilityManagerMock() = default;
    virtual ~AbilityManagerMock() = default;

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag,
        BundleInfo &bundleInfo, int32_t userId) override
    {
        bundleInfo = mockBundleInfo_;
        return getBundleInfoResult_;
    }

    ErrCode GetSignatureInfoByBundleName(const std::string &bundleName,
        SignatureInfo &signatureInfo) override
    {
        signatureInfo = mockSignatureInfo_;
        return getSignatureInfoResult_;
    }

    void SetMockBundleInfo(const BundleInfo &bundleInfo)
    {
        mockBundleInfo_ = bundleInfo;
    }

    void SetGetBundleInfoResult(bool result)
    {
        getBundleInfoResult_ = result;
    }

    void SetMockSignatureInfo(const SignatureInfo &signatureInfo)
    {
        mockSignatureInfo_ = signatureInfo;
    }

    void SetGetSignatureInfoResult(ErrCode result)
    {
        getSignatureInfoResult_ = result;
    }

private:
    BundleInfo mockBundleInfo_;
    SignatureInfo mockSignatureInfo_;
    bool getBundleInfoResult_ = true;
    ErrCode getSignatureInfoResult_ = ERR_OK;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // IABILITY_MANAGER_MOCK_H
