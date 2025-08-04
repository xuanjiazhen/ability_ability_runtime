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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_APP_CONTROL_PROXY_RESULT_CLIENT_H
#define MOCK_OHOS_ABILITY_RUNTIME_APP_CONTROL_PROXY_RESULT_CLIENT_H

#include <mutex>

#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
class AppControlProxyResultClient {
public:
    static AppControlProxyResultClient &GetInstance();
    bool GetAppControlProxyNull();
    void SetAppControlProxyNull(bool isAppControlProxyNull);

private:
    AppControlProxyResultClient() = default;
    ~AppControlProxyResultClient() = default;
    DISALLOW_COPY_AND_MOVE(AppControlProxyResultClient);
    std::mutex mutex_;
    bool isAppControlProxyNull_ = false;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME_APP_CONTROL_PROXY_RESULT_CLIENT_H