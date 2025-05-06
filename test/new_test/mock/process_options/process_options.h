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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_PROCESS_OPTIONS_H
#define MOCK_OHOS_ABILITY_RUNTIME_PROCESS_OPTIONS_H

#include "parcel.h"

namespace OHOS {
namespace AAFwk {
enum class ProcessMode {
    UNSPECIFIED = 0,
    NEW_PROCESS_ATTACH_TO_PARENT = 1,
    NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM = 2,
    ATTACH_TO_STATUS_BAR_ITEM = 3,
    NEW_HIDDEN_PROCESS = 4,
    NO_ATTACHMENT = 99,
    END
};

enum class StartupVisibility {
    UNSPECIFIED = -1,
    STARTUP_HIDE = 0,
    STARTUP_SHOW = 1,
    END
};

class ProcessOptions final : public Parcelable {
public:
    ProcessOptions() = default;
    ~ProcessOptions() = default;

    bool ReadFromParcel(Parcel &parcel)
    {
        return false;
    }

    virtual bool Marshalling(Parcel &parcel) const override
    {
        return false;
    }

    static ProcessOptions *Unmarshalling(Parcel &parcel)
    {
        return nullptr;
    }

    static ProcessMode ConvertInt32ToProcessMode(int32_t value)
    {
        return ProcessMode::UNSPECIFIED;
    }
    static StartupVisibility ConvertInt32ToStartupVisibility(int32_t value)
    {
        return StartupVisibility::UNSPECIFIED;
    }
    static bool IsNewProcessMode(ProcessMode value)
    {
        return false;
    }
    static bool IsAttachToStatusBarMode(ProcessMode value)
    {
        return false;
    }
    static bool IsValidProcessMode(ProcessMode value)
    {
        return false;
    }
    static bool IsNoAttachmentMode(ProcessMode value)
    {
        return false;
    }
    static bool IsAttachToStatusBarItemMode(ProcessMode value)
    {
        return false;
    }
    static bool IsNewHiddenProcessMode(ProcessMode value)
    {
        return false;
    }
    bool isRestartKeepAlive = false;
    bool isStartFromNDK = false;
    ProcessMode processMode = ProcessMode::UNSPECIFIED;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
