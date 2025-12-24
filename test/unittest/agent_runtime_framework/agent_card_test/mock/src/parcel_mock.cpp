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

#include "parcel_mock.h"

namespace OHOS {
// Implementation of ParcelMock methods
bool ParcelMock::WriteString(const std::string& value)
{
    return true; // Default implementation for mock
}

bool ParcelMock::WriteBool(bool value)
{
    return true; // Default implementation for mock
}

bool ParcelMock::WriteStringVector(const std::vector<std::string>& value)
{
    return true; // Default implementation for mock
}

std::string ParcelMock::ReadString()
{
    return ""; // Default implementation for mock
}

bool ParcelMock::ReadBool()
{
    return false; // Default implementation for mock
}

bool ParcelMock::ReadStringVector(std::vector<std::string>* value)
{
    if (value) {
        value->clear();
    }
    return true; // Default implementation for mock
}
} // namespace OHOS