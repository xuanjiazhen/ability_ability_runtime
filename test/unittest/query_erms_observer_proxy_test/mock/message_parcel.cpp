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

#include "message_parcel.h"

namespace OHOS {
namespace {
bool g_mockWriteInterfaceToken = true;
bool g_mockWriteString = true;
bool g_mockWriteBool = true;
bool g_mockWriteInt32 = true;
uint32_t g_writeInterfaceTokenCalls = 0;
uint32_t g_writeStringCalls = 0;
uint32_t g_writeBoolCalls = 0;
uint32_t g_writeInt32Calls = 0;
uint8_t g_mockWriteStringCount = 0;
uint8_t g_mockWriteStringMax = 0;
uint8_t g_mockWriteBoolCount = 0;
uint8_t g_mockWriteBoolMax = 0;
uint8_t g_mockWriteInt32Count = 0;
uint8_t g_mockWriteInt32Max = 0;
} // namespace

void MockWriteInterfaceToken(bool state)
{
    g_mockWriteInterfaceToken = state;
}

void MockWriteString(bool state, uint8_t count)
{
    g_mockWriteString = state;
    g_mockWriteStringMax = count;
}

void MockWriteBool(bool state, uint8_t count)
{
    g_mockWriteBool = state;
    g_mockWriteBoolMax = count;
}

void MockWriteInt32(bool state, uint8_t count)
{
    g_mockWriteInt32 = state;
    g_mockWriteInt32Max = count;
}

void ResetMessageParcelState()
{
    g_mockWriteInterfaceToken = true;
    g_mockWriteString = true;
    g_mockWriteBool = true;
    g_mockWriteInt32 = true;
    g_writeInterfaceTokenCalls = 0;
    g_writeStringCalls = 0;
    g_writeBoolCalls = 0;
    g_writeInt32Calls = 0;
    g_mockWriteStringCount = 0;
    g_mockWriteStringMax = 0;
    g_mockWriteBoolCount = 0;
    g_mockWriteBoolMax = 0;
    g_mockWriteInt32Count = 0;
    g_mockWriteInt32Max = 0;
}

uint32_t GetWriteInterfaceTokenCalls()
{
    return g_writeInterfaceTokenCalls;
}

uint32_t GetWriteStringCalls()
{
    return g_writeStringCalls;
}

uint32_t GetWriteBoolCalls()
{
    return g_writeBoolCalls;
}

uint32_t GetWriteInt32Calls()
{
    return g_writeInt32Calls;
}

bool MessageParcel::WriteInterfaceToken(const std::u16string &)
{
    g_writeInterfaceTokenCalls++;
    return g_mockWriteInterfaceToken;
}

bool MessageParcel::WriteString(const std::string &)
{
    g_writeStringCalls++;
    if (g_mockWriteStringCount < g_mockWriteStringMax) {
        g_mockWriteStringCount++;
        return !g_mockWriteString;
    }
    return g_mockWriteString;
}

bool MessageParcel::WriteBool(bool)
{
    g_writeBoolCalls++;
    if (g_mockWriteBoolCount < g_mockWriteBoolMax) {
        g_mockWriteBoolCount++;
        return !g_mockWriteBool;
    }
    return g_mockWriteBool;
}

bool MessageParcel::WriteInt32(int32_t)
{
    g_writeInt32Calls++;
    if (g_mockWriteInt32Count < g_mockWriteInt32Max) {
        g_mockWriteInt32Count++;
        return !g_mockWriteInt32;
    }
    return g_mockWriteInt32;
}
} // namespace OHOS
