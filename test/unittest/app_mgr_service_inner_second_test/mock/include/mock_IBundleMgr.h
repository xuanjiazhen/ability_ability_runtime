/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_MOCK_I_BUNDLE_MGR_H
#define FOUNDATION_MOCK_I_BUNDLE_MGR_H

#include "bundle_mgr_interface.h"
#include "gmock/gmock.h"

namespace OHOS::AppExecFwk {
class MOCKIBundleMgr : public IBundleMgr {
public:
    MOCKIBundleMgr() = default;
    ~MOCKIBundleMgr() override {};

    MOCK_METHOD0(GetOverlayManagerProxy, sptr<IOverlayManager>());
    
    sptr<IRemoteObject> AsObject();
};
} // namespace OHOS::AppExecFwk
#endif // FOUNDATION_MOCK_I_BUNDLE_MGR_H
