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

#ifndef MOCK_OHOS_ROSEN_WINDOW_MANAGER_H
#define MOCK_OHOS_ROSEN_WINDOW_MANAGER_H

#include <memory>
#include <refbase.h>
#include <vector>
#include <iremote_object.h>
#include "wm_common.h"
#include "mock_common.h"

namespace OHOS {
namespace Rosen {
class WindowManager {
public:
    static WindowManager& GetInstance()
    {
        static WindowManager instance;
        return instance;
    }

    WMError RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener)
    {
        return WMError::WM_OK;
    }

    WMError RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
    {
        return WMError::WM_OK;
    }

    WMError UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener)
    {
        return WMError::WM_OK;
    }

    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = 0) {}

    WMError DumpSessionAll(std::vector<std::string>& infos)
    {
        return WMError::WM_OK;
    }

    WMError DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos)
    {
        return WMError::WM_OK;
    }
};
} // namespace Rosen
} // namespace OHOS
#endif
