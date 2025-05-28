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

#ifndef OHOS_AAFWK_ACQUIRESHAREDATACALLBACKPROXY_H
#define OHOS_AAFWK_ACQUIRESHAREDATACALLBACKPROXY_H

#include <iremote_proxy.h>
#include "iacquire_share_data_callback.h"

namespace OHOS {
namespace AAFwk {

class AcquireShareDataCallbackProxy : public IRemoteProxy<IAcquireShareDataCallback> {
public:
    explicit AcquireShareDataCallbackProxy(
        const sptr<IRemoteObject>& remote)
        : IRemoteProxy<IAcquireShareDataCallback>(remote)
    {}

    virtual ~AcquireShareDataCallbackProxy()
    {}

    ErrCode AcquireShareDataDone(
        int32_t resultCode,
        const WantParams& wantParam,
        int32_t& funcResult) override;

private:
    static inline BrokerDelegator<AcquireShareDataCallbackProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFWK_ACQUIRESHAREDATACALLBACKPROXY_H