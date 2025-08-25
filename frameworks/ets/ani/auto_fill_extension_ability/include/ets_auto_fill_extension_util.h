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

#ifndef OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_UTIL_H
#define OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_UTIL_H

#include "ani.h"
#include "view_data.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
struct FillResponse {
    AbilityBase::ViewData viewData;
};

struct PopupSize {
    int32_t width = 0;
    int32_t height = 0;
};

enum AutoFillCommand {
    NONE,
    FILL,
    SAVE,
    UPDATE,
    RESIZE,
    INPUT,
    RELOAD_IN_MODAL
};

struct CustomData {
    AAFwk::WantParams data;
};

class EtsAutoFillExtensionUtil {
public:
    static void UnwrapViewData(ani_env *env, const ani_object object, AbilityBase::ViewData &viewData);
    static void UnwrapPageNodeInfo(ani_env *env, const ani_object object, AbilityBase::PageNodeInfo &node);
    static void UnwrapRectData(ani_env *env, const ani_object object, AbilityBase::Rect &rect);
    static void UnwrapFillResponse(ani_env *env, const ani_object object, FillResponse &response);
    static void UnwrapPopupSize(ani_env *env, const ani_object object, PopupSize &popupSize);

    enum AutoFillResultCode {
        CALLBACK_SUCESS = 0,
        CALLBACK_FAILED,
        CALLBACK_CANCEL,
        CALLBACK_REMOVE_TIME_OUT,
        CALLBACK_FAILED_INVALID_PARAM,
    };

private:
    static void UnwrapViewDataString(ani_env *env, const ani_object object, AbilityBase::ViewData &viewData);
    static void UnwrapViewDataBoolean(ani_env *env, const ani_object object, AbilityBase::ViewData &viewData);
    static void UnwrapPageNodeInfoString(ani_env *env, const ani_object object, AbilityBase::PageNodeInfo &node);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_UTIL_H