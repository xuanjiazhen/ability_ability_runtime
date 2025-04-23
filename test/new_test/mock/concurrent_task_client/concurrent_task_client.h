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

#ifndef MOCK_CONCURRENT_TASK_SERVICE_INTERFACES_INNERAPI_CONCURRENT_TASK_CLIENT_INCLUDE_CONCURRENT_TASK_CLIENT_H
#define MOCK_CONCURRENT_TASK_SERVICE_INTERFACES_INNERAPI_CONCURRENT_TASK_CLIENT_INCLUDE_CONCURRENT_TASK_CLIENT_H

#include <unordered_map>
#include <string>

namespace OHOS {
namespace ConcurrentTask {

class ConcurrentTaskClient {
public:

    static ConcurrentTaskClient& GetInstance()
    {
        static ConcurrentTaskClient instance;
        return instance;
    }

    void RequestAuth(const std::unordered_map<std::string, std::string>& mapPayload) {}
};
} // namespace ConcurrentTask
} // namespace OHOS

#endif
