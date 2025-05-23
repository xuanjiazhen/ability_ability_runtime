/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_IPC_IREMOTE_BROKER_H
#define MOCK_OHOS_IPC_IREMOTE_BROKER_H

#include <unordered_map>
#include <functional>
#include <vector>
#include "iremote_object.h"
#include "refbase.h"

namespace OHOS {
class IRemoteBroker : public virtual RefBase {
public:
    IRemoteBroker() = default;
    virtual ~IRemoteBroker() override = default;
    virtual sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};

#define DECLARE_INTERFACE_DESCRIPTOR(DESCRIPTOR)                         \
    static constexpr const char16_t *metaDescriptor_ = DESCRIPTOR;       \
    static inline const std::u16string GetDescriptor()                   \
    {                                                                    \
        return metaDescriptor_;                                          \
    }

template <typename INTERFACE> inline sptr<INTERFACE> iface_cast(const sptr<IRemoteObject> &object)
{
    return nullptr;
}
} // namespace OHOS
#endif
 