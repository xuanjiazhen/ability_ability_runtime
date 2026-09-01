/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "process_info.h"

#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
ProcessInfo::ProcessInfo(const std::string &name, const pid_t &pid) : processName_(name), pid_(pid)
{}


bool ProcessInfo::ReadFromParcel(Parcel &parcel)
{
    processName_ = Str16ToStr8(parcel.ReadString16());
    pid_ = parcel.ReadInt32();
    processType_ = ProcessType(parcel.ReadInt32());
    return true;
}


ProcessInfo *ProcessInfo::Unmarshalling(Parcel &parcel)
{
    ProcessInfo *processInfo = new (std::nothrow) ProcessInfo();
    if (processInfo && !processInfo->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ProcessInfo::Unmarshalling ReadFromParcel failed");
        delete processInfo;
        processInfo = nullptr;
    }
    return processInfo;
}


bool ProcessInfo::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteString16(Str8ToStr16(processName_)) && parcel.WriteInt32(pid_)
        && parcel.WriteInt32(static_cast<int32_t>(processType_)));
}

void ProcessInfo::SetProcessType(const ProcessType &processType)
{
    processType_ = processType;
}

ProcessType ProcessInfo::GetProcessType() const
{
    return processType_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
