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

#ifndef POLICY_RAW_DATA_H
#define POLICY_RAW_DATA_H

#include "securec.h"

namespace OHOS {
namespace AccessControl {
namespace SandboxManager {
class UriPermissionRawData {
public:
// struct UriPermissionRawData {
    uint32_t size;
    const void* data;

    UriPermissionRawData() : size(0), data(nullptr) {}
    int32_t RawDataCpy(const void* readdata)
    {
        if (readdata == nullptr || size == 0) {
            return -1;
        }
        void* newData = malloc(size);
        if (newData == nullptr) {
            return -1;
        }
        if (memcpy_s(newData, size, readdata, size) != EOK) {
            free(newData);
            return -1;
        }
        if (data != nullptr) {
            free(const_cast<void*>(data));
        }
        data = newData;
        return 0;
    }
    // 析构函数
    ~UriPermissionRawData()
    {
        if (data != nullptr) {
            free(const_cast<void*>(data));
        }
    }
};
} // namespace SandboxManager
} // namespace AccessControl
} // namespace OHOS
#endif // POLICY_RAW_DATA_H