/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "launch_service_info.h"

#include <cstdlib>
#include <limits>
#include <memory>

#include "message_parcel.h"
#include "securec.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint32_t MAX_INFORMATION_COUNT = 1024;
constexpr uint32_t MAX_RAW_DATA_SIZE = 4 * 1024 * 1024;
constexpr size_t MAX_DESCRIPTOR_FIELD_LENGTH = 512;

bool IsValidState(int32_t state) {
  return state >= static_cast<int32_t>(LaunchServiceState::NOT_REGISTERED) &&
         state <= static_cast<int32_t>(LaunchServiceState::DISABLED);
}

bool IsValidProcessState(int32_t state) {
  return state >= static_cast<int32_t>(LaunchServiceProcessState::CREATED) &&
         state <= static_cast<int32_t>(LaunchServiceProcessState::TERMINATED);
}
} // namespace

bool LaunchServiceDescriptor::ReadFromParcel(Parcel &parcel) {
  if (!parcel.ReadString(bundleName) || !parcel.ReadString(moduleName) ||
      !parcel.ReadString(serviceName) || !parcel.ReadBool(hasUserId) ||
      (hasUserId && !parcel.ReadInt32(userId)) ||
      !parcel.ReadBool(hasAppIndex) ||
      (hasAppIndex && !parcel.ReadInt32(appIndex))) {
    return false;
  }
  return IsValid();
}

bool LaunchServiceDescriptor::Marshalling(Parcel &parcel) const {
  return IsValid() && parcel.WriteString(bundleName) &&
         parcel.WriteString(moduleName) && parcel.WriteString(serviceName) &&
         parcel.WriteBool(hasUserId) &&
         (!hasUserId || parcel.WriteInt32(userId)) &&
         parcel.WriteBool(hasAppIndex) &&
         (!hasAppIndex || parcel.WriteInt32(appIndex));
}

bool LaunchServiceDescriptor::IsValid() const {
  return !bundleName.empty() &&
         bundleName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
         !moduleName.empty() &&
         moduleName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
         !serviceName.empty() &&
         serviceName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
         (!hasUserId || userId >= 0) && (!hasAppIndex || appIndex >= 0);
}

LaunchServiceDescriptor *
LaunchServiceDescriptor::Unmarshalling(Parcel &parcel) {
  auto descriptor = std::make_unique<LaunchServiceDescriptor>();
  if (!descriptor->ReadFromParcel(parcel)) {
    return nullptr;
  }
  return descriptor.release();
}

bool LaunchServiceInformation::ReadFromParcel(Parcel &parcel) {
  std::unique_ptr<LaunchServiceDescriptor> descriptorValue(
      parcel.ReadParcelable<LaunchServiceDescriptor>());
  int32_t stateValue = 0;
  if (descriptorValue == nullptr || !parcel.ReadInt32(stateValue) ||
      !IsValidState(stateValue) || !parcel.ReadInt32(pid) || pid < 0) {
    return false;
  }
  descriptor = *descriptorValue;
  state = static_cast<LaunchServiceState>(stateValue);
  return state != LaunchServiceState::NOT_REGISTERED || pid == 0;
}

bool LaunchServiceInformation::Marshalling(Parcel &parcel) const {
  if (!IsValidState(static_cast<int32_t>(state)) || pid < 0 ||
      (state == LaunchServiceState::NOT_REGISTERED && pid != 0)) {
    return false;
  }
  return parcel.WriteParcelable(&descriptor) &&
         parcel.WriteInt32(static_cast<int32_t>(state)) &&
         parcel.WriteInt32(pid);
}

LaunchServiceInformation *
LaunchServiceInformation::Unmarshalling(Parcel &parcel) {
  auto information = std::make_unique<LaunchServiceInformation>();
  if (!information->ReadFromParcel(parcel)) {
    return nullptr;
  }
  return information.release();
}

bool LaunchServiceStateData::ReadFromParcel(Parcel &parcel) {
  std::unique_ptr<LaunchServiceDescriptor> descriptorValue(
      parcel.ReadParcelable<LaunchServiceDescriptor>());
  int32_t stateValue = 0;
  if (descriptorValue == nullptr || !parcel.ReadInt32(stateValue) ||
      !IsValidState(stateValue)) {
    return false;
  }
  descriptor = *descriptorValue;
  state = static_cast<LaunchServiceState>(stateValue);
  return true;
}

bool LaunchServiceStateData::Marshalling(Parcel &parcel) const {
  return descriptor.IsValid() && IsValidState(static_cast<int32_t>(state)) &&
         parcel.WriteParcelable(&descriptor) &&
         parcel.WriteInt32(static_cast<int32_t>(state));
}

LaunchServiceStateData *LaunchServiceStateData::Unmarshalling(Parcel &parcel) {
  auto data = std::make_unique<LaunchServiceStateData>();
  return data->ReadFromParcel(parcel) ? data.release() : nullptr;
}

bool LaunchServiceProcessStateData::ReadFromParcel(Parcel &parcel) {
  std::unique_ptr<LaunchServiceDescriptor> descriptorValue(
      parcel.ReadParcelable<LaunchServiceDescriptor>());
  int32_t stateValue = 0;
  if (descriptorValue == nullptr || !parcel.ReadInt32(pid) || pid < 0 ||
      !parcel.ReadInt32(stateValue) || !IsValidProcessState(stateValue)) {
    return false;
  }
  descriptor = *descriptorValue;
  state = static_cast<LaunchServiceProcessState>(stateValue);
  return true;
}

bool LaunchServiceProcessStateData::Marshalling(Parcel &parcel) const {
  return descriptor.IsValid() && pid >= 0 &&
         IsValidProcessState(static_cast<int32_t>(state)) &&
         parcel.WriteParcelable(&descriptor) && parcel.WriteInt32(pid) &&
         parcel.WriteInt32(static_cast<int32_t>(state));
}

LaunchServiceProcessStateData *
LaunchServiceProcessStateData::Unmarshalling(Parcel &parcel) {
  auto data = std::make_unique<LaunchServiceProcessStateData>();
  return data->ReadFromParcel(parcel) ? data.release() : nullptr;
}

LaunchServiceInformationsRawData::~LaunchServiceInformationsRawData() {
  if (isMalloc && data != nullptr) {
    free(const_cast<void *>(data));
  }
}

int32_t LaunchServiceInformationsRawData::RawDataCpy(const void *readData) {
  if (readData == nullptr || size == 0 || size > MAX_RAW_DATA_SIZE) {
    return -1;
  }
  void *newData = malloc(size);
  if (newData == nullptr || memcpy_s(newData, size, readData, size) != EOK) {
    free(newData);
    return -1;
  }
  if (isMalloc && data != nullptr) {
    free(const_cast<void *>(data));
  }
  data = newData;
  isMalloc = true;
  return 0;
}

bool LaunchServiceInformationsRawData::FromVector(
    const std::vector<LaunchServiceInformation> &informations,
    LaunchServiceInformationsRawData &rawData) {
  if (informations.size() > MAX_INFORMATION_COUNT) {
    return false;
  }
  MessageParcel parcel;
  if (!parcel.WriteUint32(static_cast<uint32_t>(informations.size()))) {
    return false;
  }
  for (const auto &information : informations) {
    if (!parcel.WriteParcelable(&information)) {
      return false;
    }
  }
  size_t dataSize = parcel.GetDataSize();
  if (dataSize == 0 || dataSize > MAX_RAW_DATA_SIZE ||
      dataSize > std::numeric_limits<uint32_t>::max()) {
    return false;
  }
  rawData.ownedData.assign(reinterpret_cast<const char *>(parcel.GetData()),
                           dataSize);
  rawData.data = rawData.ownedData.data();
  rawData.size = static_cast<uint32_t>(dataSize);
  rawData.isMalloc = false;
  return true;
}

bool LaunchServiceInformationsRawData::ToVector(
    const LaunchServiceInformationsRawData &rawData,
    std::vector<LaunchServiceInformation> &informations) {
  if (rawData.data == nullptr || rawData.size == 0 ||
      rawData.size > MAX_RAW_DATA_SIZE) {
    return false;
  }
  MessageParcel parcel;
  if (!parcel.ParseFrom(reinterpret_cast<uintptr_t>(rawData.data),
                        rawData.size)) {
    return false;
  }
  uint32_t count = 0;
  if (!parcel.ReadUint32(count) || count > MAX_INFORMATION_COUNT) {
    return false;
  }
  std::vector<LaunchServiceInformation> values;
  values.reserve(count);
  for (uint32_t index = 0; index < count; ++index) {
    std::unique_ptr<LaunchServiceInformation> information(
        parcel.ReadParcelable<LaunchServiceInformation>());
    if (information == nullptr) {
      return false;
    }
    values.emplace_back(*information);
  }
  informations = std::move(values);
  return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
