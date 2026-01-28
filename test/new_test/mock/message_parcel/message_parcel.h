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
#include <string>

#include "parcel.h"

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

inline void ResetMessageParcelState()
{
	MockWriteInterfaceTokenState() = true;
	MockWriteStringState() = true;
	MockWriteBoolState() = true;
	MockWriteInt32State() = true;
	MockWriteStringMax() = 0;
	MockWriteBoolMax() = 0;
	MockWriteInt32Max() = 0;
	MockWriteStringCount() = 0;
	MockWriteBoolCount() = 0;
	MockWriteInt32Count() = 0;
	WriteInterfaceTokenCalls() = 0;
	WriteStringCalls() = 0;
	WriteBoolCalls() = 0;
	WriteInt32Calls() = 0;
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
};
} // namespace OHOS
#endif // MOCK_OHOS_IPC_MESSAGE_PARCEL_H
