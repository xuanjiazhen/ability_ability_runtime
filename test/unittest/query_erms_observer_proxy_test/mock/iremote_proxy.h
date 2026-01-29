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

#ifndef MOCK_OHOS_QUERY_ERMS_IREMOTE_PROXY_H
#define MOCK_OHOS_QUERY_ERMS_IREMOTE_PROXY_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
template <typename T>
class BrokerDelegator {
public:
    BrokerDelegator() = default;
    ~BrokerDelegator() = default;
};

template <typename INTERFACE> class IRemoteProxy : public INTERFACE {
public:
    explicit IRemoteProxy(const sptr<IRemoteObject> &impl) : remote_(impl) {}
    virtual ~IRemoteProxy() = default;

    sptr<IRemoteObject> Remote()
    {
        return remote_;
    }

private:
    sptr<IRemoteObject> remote_;
};
} // namespace OHOS
#endif // MOCK_OHOS_QUERY_ERMS_IREMOTE_PROXY_H
