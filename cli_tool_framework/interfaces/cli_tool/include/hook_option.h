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

#ifndef OHOS_ABILITY_RUNTIME_HOOK_OPTION_H
#define OHOS_ABILITY_RUNTIME_HOOK_OPTION_H

#include "parcel.h"

namespace OHOS {
namespace CliTool {

class HookOption : public Parcelable {
public:
    int32_t dmSessionId = -1;
    int32_t toolCallId = -1;

    bool Marshalling(Parcel &parcel) const override;
    static HookOption *Unmarshalling(Parcel &parcel);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_HOOK_OPTION_H
