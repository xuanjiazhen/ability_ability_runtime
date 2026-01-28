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

#ifndef MOCK_OHOS_QUERY_ERMS_MESSAGE_PARCEL_H
#define MOCK_OHOS_QUERY_ERMS_MESSAGE_PARCEL_H

#include <cstdint>
#include <string>

namespace OHOS {
void MockWriteInterfaceToken(bool state);
void MockWriteString(bool state, uint8_t count);
void MockWriteBool(bool state, uint8_t count);
void MockWriteInt32(bool state, uint8_t count);
void ResetMessageParcelState();
uint32_t GetWriteInterfaceTokenCalls();
uint32_t GetWriteStringCalls();
uint32_t GetWriteBoolCalls();
uint32_t GetWriteInt32Calls();

class MessageParcel {
public:
    MessageParcel() = default;
    ~MessageParcel() = default;

    bool WriteInterfaceToken(const std::u16string &token);
    bool WriteString(const std::string &value);
    bool WriteBool(bool value);
    bool WriteInt32(int32_t value);
};
} // namespace OHOS
#endif // MOCK_OHOS_QUERY_ERMS_MESSAGE_PARCEL_H
