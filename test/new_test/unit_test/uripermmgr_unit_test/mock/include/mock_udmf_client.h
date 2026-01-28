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

#ifndef MOCK_UDMF_CLIENT_H
#define MOCK_UDMF_CLIENT_H

#include "oh_mock_utils.h"
#include <string>
#include <vector>

#ifdef ABILITY_RUNTIME_UDMF_ENABLE

namespace OHOS {
namespace UDMF {

struct UnifiedData {
    std::string key;
    std::vector<std::string> records;
};

struct UnifiedRecord {
    std::string typeId;
    std::string content;
};

struct Privilege {
    std::string tokenID;
    uint32_t flag;
};

struct QueryOption {
    std::string key;
};

class MockUdmfClient {
public:
    OH_MOCK_METHOD(int32_t, MockUdmfClient, GetBatchData, const std::vector<std::string>& keys,
                   std::vector<UnifiedData>& data);

    OH_MOCK_METHOD(int32_t, MockUdmfClient, AddPrivilege, const std::string& key, const Privilege& privilege);

    OH_MOCK_METHOD(bool, MockUdmfClient, ProcessUdmfKey, const std::string& key, std::string& processedKey);

    OH_MOCK_METHOD(int32_t, MockUdmfClient, SetData, const UnifiedData& data);

    OH_MOCK_METHOD(int32_t, MockUdmfClient, GetData, const QueryOption& option, std::vector<UnifiedData>& data);
};

} // namespace UDMF
} // namespace OHOS

#endif // ABILITY_RUNTIME_UDMF_ENABLE

#endif // MOCK_UDMF_CLIENT_H
