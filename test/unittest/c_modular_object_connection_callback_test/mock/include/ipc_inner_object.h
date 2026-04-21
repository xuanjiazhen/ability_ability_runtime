/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_IPC_INNER_OBJECT_H
#define MOCK_IPC_INNER_OBJECT_H

#include <new>

namespace OHOS {
class IRemoteObject;
} // namespace OHOS

struct OHIPCRemoteProxy {
    OHOS::sptr<OHOS::IRemoteObject> remote;
};

struct OHIPCRemoteStub {
    OHOS::sptr<OHOS::IRemoteObject> remote;
};

static inline OHIPCRemoteProxy *CreateIPCRemoteProxy(OHOS::sptr<OHOS::IRemoteObject> &object)
{
    if (object == nullptr) {
        return nullptr;
    }
    auto *proxy = new (std::nothrow) OHIPCRemoteProxy();
    if (proxy != nullptr) {
        proxy->remote = object;
    }
    return proxy;
}

#endif // MOCK_IPC_INNER_OBJECT_H
