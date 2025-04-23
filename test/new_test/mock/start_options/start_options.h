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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_START_OPTIONS_H
#define MOCK_OHOS_ABILITY_RUNTIME_START_OPTIONS_H

#include <string>

#include "ability_info.h"
#include "parcel.h"
#include "process_options.h"

namespace OHOS {
namespace AAFwk {
class StartWindowOption {
public:
    bool hasStartWindow = false;
    std::string startWindowBackgroundColor;
};

class StartOptions final : public Parcelable, public std::enable_shared_from_this<StartOptions> {
public:
    std::shared_ptr<ProcessOptions> processOptions = nullptr;
    std::shared_ptr<StartWindowOption> startWindowOption = nullptr;
    std::vector<AppExecFwk::SupportWindowMode> supportWindowModes_;
    std::string requestId_;
    bool ReadFromParcel(Parcel &parcel)
    {
        return false;
    }
    virtual bool Marshalling(Parcel &parcel) const override
    {
        return false;
    }
    static StartOptions *Unmarshalling(Parcel &parcel)
    {
        return nullptr;
    }

    int32_t GetWindowMode() const
    {
        return 0;
    }

    int32_t GetDisplayID() const
    {
        return 0;
    }

    int32_t GetWindowFocused() const
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
