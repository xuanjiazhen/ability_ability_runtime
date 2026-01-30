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

#ifndef MOCK_OHOS_IPC_MESSAGE_PARCEL_H
#define MOCK_OHOS_IPC_MESSAGE_PARCEL_H

#include <cstdint>
#include <new>
#include <string>

#include "parcel.h"
#include "refbase.h"

namespace OHOS {
inline bool &MockWriteInterfaceTokenState()
{
    static bool value = true;
    return value;
}

inline bool &MockWriteStringState()
{
    static bool value = true;
    return value;
}

inline bool &MockWriteBoolState()
{
    static bool value = true;
    return value;
}

inline bool &MockWriteInt32State()
{
    static bool value = true;
    return value;
}

inline bool &MockWriteParcelableState()
{
    static bool value = true;
    return value;
}

inline uint8_t &MockWriteStringMax()
{
    static uint8_t value = 0;
    return value;
}

inline uint8_t &MockWriteBoolMax()
{
    static uint8_t value = 0;
    return value;
}

inline uint8_t &MockWriteInt32Max()
{
    static uint8_t value = 0;
    return value;
}

inline uint8_t &MockWriteParcelableMax()
{
    static uint8_t value = 0;
    return value;
}

inline uint8_t &MockWriteStringCount()
{
    static uint8_t value = 0;
    return value;
}

inline uint8_t &MockWriteBoolCount()
{
    static uint8_t value = 0;
    return value;
}

inline uint8_t &MockWriteInt32Count()
{
    static uint8_t value = 0;
    return value;
}

inline uint8_t &MockWriteParcelableCount()
{
    static uint8_t value = 0;
    return value;
}

inline uint32_t &WriteInterfaceTokenCalls()
{
    static uint32_t value = 0;
    return value;
}

inline uint32_t &WriteStringCalls()
{
    static uint32_t value = 0;
    return value;
}

inline uint32_t &WriteBoolCalls()
{
    static uint32_t value = 0;
    return value;
}

inline uint32_t &WriteInt32Calls()
{
    static uint32_t value = 0;
    return value;
}

inline uint32_t &WriteParcelableCalls()
{
    static uint32_t value = 0;
    return value;
}

inline int32_t &MockReadInt32Value()
{
    static int32_t value = 0;
    return value;
}

inline bool &MockReadParcelableState()
{
    static bool value = true;
    return value;
}

template <typename T>
inline sptr<T> &MockReadParcelableValue()
{
    static sptr<T> value;
    return value;
}

inline void MockWriteInterfaceToken(bool state)
{
    MockWriteInterfaceTokenState() = state;
}

inline void MockWriteString(bool state, uint8_t count)
{
    MockWriteStringState() = state;
    MockWriteStringMax() = count;
}

inline void MockWriteBool(bool state, uint8_t count)
{
    MockWriteBoolState() = state;
    MockWriteBoolMax() = count;
}

inline void MockWriteInt32(bool state, uint8_t count)
{
    MockWriteInt32State() = state;
    MockWriteInt32Max() = count;
}

inline void MockWriteParcelable(bool state, uint8_t count)
{
    MockWriteParcelableState() = state;
    MockWriteParcelableMax() = count;
}

inline void MockReadInt32(int32_t value)
{
    MockReadInt32Value() = value;
}

inline void MockReadParcelable(bool state)
{
    MockReadParcelableState() = state;
}

template <typename T>
inline void SetReadParcelableValue(const sptr<T> &value)
{
    MockReadParcelableValue<T>() = value;
}

inline void ResetMessageParcelState()
{
    MockWriteInterfaceTokenState() = MockWriteStringState() = MockWriteBoolState() = true;
    MockWriteInt32State() = MockWriteParcelableState() = MockReadParcelableState() = true;
    MockWriteStringMax() = MockWriteBoolMax() = MockWriteInt32Max() = MockWriteParcelableMax() = 0;
    MockWriteStringCount() = MockWriteBoolCount() = MockWriteInt32Count() = MockWriteParcelableCount() = 0;
    WriteInterfaceTokenCalls() = WriteStringCalls() = WriteBoolCalls() = 0;
    WriteInt32Calls() = WriteParcelableCalls() = MockReadInt32Value() = 0;
}

inline uint32_t GetWriteInterfaceTokenCalls()
{
    return WriteInterfaceTokenCalls();
}

inline uint32_t GetWriteStringCalls()
{
    return WriteStringCalls();
}

inline uint32_t GetWriteBoolCalls()
{
    return WriteBoolCalls();
}

inline uint32_t GetWriteInt32Calls()
{
    return WriteInt32Calls();
}

inline uint32_t GetWriteParcelableCalls()
{
    return WriteParcelableCalls();
}

class MessageParcel : public Parcel {
public:
    bool WriteInterfaceToken(const std::u16string &)
    {
        WriteInterfaceTokenCalls()++;
        return MockWriteInterfaceTokenState();
    }

    bool WriteString(const std::string &)
    {
        WriteStringCalls()++;
        if (MockWriteStringCount() < MockWriteStringMax()) {
            MockWriteStringCount()++;
            return !MockWriteStringState();
        }
        return MockWriteStringState();
    }

    bool WriteBool(bool)
    {
        WriteBoolCalls()++;
        if (MockWriteBoolCount() < MockWriteBoolMax()) {
            MockWriteBoolCount()++;
            return !MockWriteBoolState();
        }
        return MockWriteBoolState();
    }

    bool WriteInt32(int32_t)
    {
        WriteInt32Calls()++;
        if (MockWriteInt32Count() < MockWriteInt32Max()) {
            MockWriteInt32Count()++;
            return !MockWriteInt32State();
        }
        return MockWriteInt32State();
    }

    template <typename T>
    bool WriteParcelable(const T *)
    {
        WriteParcelableCalls()++;
        if (MockWriteParcelableCount() < MockWriteParcelableMax()) {
            MockWriteParcelableCount()++;
            return !MockWriteParcelableState();
        }
        return MockWriteParcelableState();
    }

    int32_t ReadInt32()
    {
        return MockReadInt32Value();
    }

    template <typename T>
    sptr<T> ReadParcelable()
    {
        if (!MockReadParcelableState()) {
            return nullptr;
        }
        auto preset = MockReadParcelableValue<T>();
        if (preset != nullptr) {
            return preset;
        }
        return sptr<T>(new (std::nothrow) T());
    }

    void SetMaxCapacity(int32_t cap)
    {
        maxCapacity_ = cap;
    }

    int32_t GetMaxCapacity() const
    {
        return maxCapacity_;
    }

private:
    int32_t maxCapacity_ = 0;
};
} // namespace OHOS
#endif // MOCK_OHOS_IPC_MESSAGE_PARCEL_H
