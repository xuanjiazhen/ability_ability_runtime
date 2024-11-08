/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "fault_data.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool FaultData::ReadFromParcel(Parcel &parcel)
{
    std::string strValue;
    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "name read string failed");
        return false;
    }
    errorObject.name = strValue;

    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "message read string failed");
        return false;
    }
    errorObject.message = strValue;

    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "stack read string failed");
        return false;
    }
    errorObject.stack = strValue;

    int type = 0;
    if (!parcel.ReadInt32(type)) {
        TAG_LOGE(AAFwkTag::APPMGR, "faultType read int32 failed");
        return false;
    }
    faultType = static_cast<FaultDataType>(type);

    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "timeoutMarkers read string failed");
        return false;
    }
    timeoutMarkers = strValue;

    waitSaveState = parcel.ReadBool();
    notifyApp = parcel.ReadBool();
    forceExit = parcel.ReadBool();
    state = parcel.ReadUint32();
    eventId = parcel.ReadInt32();
    if (parcel.ReadBool()) {
        token = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }
    return true;
}

FaultData *FaultData::Unmarshalling(Parcel &parcel)
{
    FaultData *info = new FaultData();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool FaultData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(errorObject.name)) {
        TAG_LOGE(AAFwkTag::APPMGR, "name [%{public}s] write string failed", errorObject.name.c_str());
        return false;
    }

    if (!parcel.WriteString(errorObject.message)) {
        TAG_LOGE(AAFwkTag::APPMGR, "message [%{public}s] write string failed", errorObject.message.c_str());
        return false;
    }
    
    if (!parcel.WriteString(errorObject.stack)) {
        TAG_LOGE(AAFwkTag::APPMGR, "stack [%{public}s] write string failed", errorObject.stack.c_str());
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(faultType))) {
        TAG_LOGE(AAFwkTag::APPMGR, "faultType [%{public}d] write int32 failed", static_cast<int32_t>(faultType));
        return false;
    }

    if (!parcel.WriteString(timeoutMarkers)) {
        TAG_LOGE(AAFwkTag::APPMGR, "timeoutMarkers [%{public}s] write string failed", timeoutMarkers.c_str());
        return false;
    }

    if (!parcel.WriteBool(waitSaveState)) {
        TAG_LOGE(AAFwkTag::APPMGR, "waitSaveState [%{public}s] write bool failed", waitSaveState ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(notifyApp)) {
        TAG_LOGE(AAFwkTag::APPMGR, "notifyApp [%{public}s] write bool failed", notifyApp ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(forceExit)) {
        TAG_LOGE(AAFwkTag::APPMGR, "forceExit [%{public}s] write bool failed", forceExit ? "true" : "false");
        return false;
    }

    if (!parcel.WriteUint32(state)) {
        TAG_LOGE(AAFwkTag::APPMGR, "state [%{public}u] write uint32 failed", state);
        return false;
    }

    if (!parcel.WriteInt32(eventId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "eventId [%{public}u] write int32 failed", eventId);
        return false;
    }

    if (token == nullptr) {
        if (!parcel.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "token flage [false] write bool failed");
            return false;
        }
    } else {
        if (!parcel.WriteBool(true) || !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(token)) {
            TAG_LOGE(AAFwkTag::APPMGR, "token flage [true] write bool failed");
            return false;
        }
    }
    return true;
}

bool AppFaultDataBySA::ReadFromParcel(Parcel &parcel)
{
    std::string strValue;
    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read string failed");
        return false;
    }
    errorObject.name = strValue;

    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read string failed");
        return false;
    }
    errorObject.message = strValue;

    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read string failed");
        return false;
    }
    errorObject.stack = strValue;

    int type = 0;
    if (!parcel.ReadInt32(type)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read int32 failed");
        return false;
    }
    faultType = static_cast<FaultDataType>(type);

    if (!parcel.ReadInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read int32 failed");
        return false;
    }

    if (!parcel.ReadString(strValue)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read string failed");
        return false;
    }
    timeoutMarkers = strValue;

    waitSaveState = parcel.ReadBool();
    notifyApp = parcel.ReadBool();
    forceExit = parcel.ReadBool();
    state = parcel.ReadUint32();
    eventId = parcel.ReadInt32();
    if (parcel.ReadBool()) {
        token = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }
    return true;
}

AppFaultDataBySA *AppFaultDataBySA::Unmarshalling(Parcel &parcel)
{
    AppFaultDataBySA *info = new AppFaultDataBySA();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool AppFaultDataBySA::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(errorObject.name)) {
        TAG_LOGE(AAFwkTag::APPMGR, "name [%{public}s] write string failed", errorObject.name.c_str());
        return false;
    }

    if (!parcel.WriteString(errorObject.message)) {
        TAG_LOGE(AAFwkTag::APPMGR, "message [%{public}s] write string failed", errorObject.message.c_str());
        return false;
    }
    
    if (!parcel.WriteString(errorObject.stack)) {
        TAG_LOGE(AAFwkTag::APPMGR, "stack [%{public}s] write string failed", errorObject.stack.c_str());
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(faultType))) {
        TAG_LOGE(AAFwkTag::APPMGR, "faultType [%{public}d] write int32 failed", static_cast<int32_t>(faultType));
        return false;
    }

    if (!parcel.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid [%{public}d] write int32 failed", static_cast<int32_t>(pid));
        return false;
    }

    if (!parcel.WriteString(timeoutMarkers)) {
        TAG_LOGE(AAFwkTag::APPMGR, "timeoutMarkers [%{public}s] write string failed", timeoutMarkers.c_str());
        return false;
    }

    if (!parcel.WriteBool(waitSaveState)) {
        TAG_LOGE(AAFwkTag::APPMGR, "waitSaveState [%{public}s] write bool failed", waitSaveState ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(notifyApp)) {
        TAG_LOGE(AAFwkTag::APPMGR, "notifyApp [%{public}s] write bool failed", notifyApp ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(forceExit)) {
        TAG_LOGE(AAFwkTag::APPMGR, "forceExit [%{public}s] write bool failed", forceExit ? "true" : "false");
        return false;
    }

    if (!parcel.WriteUint32(state)) {
        TAG_LOGE(AAFwkTag::APPMGR, "state [%{public}u] write uint32 failed", state);
        return false;
    }

    if (!parcel.WriteInt32(eventId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "eventId [%{public}u] write int32 failed", eventId);
        return false;
    }

    if (token == nullptr) {
        if (!parcel.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "token flage [false] write bool failed");
            return false;
        }
    } else {
        if (!parcel.WriteBool(true) || !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(token)) {
            TAG_LOGE(AAFwkTag::APPMGR, "token flage [true] write bool failed");
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS