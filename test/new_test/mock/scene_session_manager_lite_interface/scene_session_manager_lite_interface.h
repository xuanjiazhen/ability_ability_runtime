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

#ifndef MOCK_OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_INTERFACE_H
#define MOCK_OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_INTERFACE_H

#include "element_name.h"
#include "extension_ability_info.h"
#include "mock_common.h"
#include "wm_common.h"

namespace OHOS::Rosen {
class ISceneSessionManagerLite : public IRemoteBroker {
public:
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = 0) {}
    WMError CheckUIExtensionCreation(int32_t windowId, uint32_t tokenId, const AppExecFwk::ElementName& element,
        AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid)
    {
        return WMError::WM_OK;
    }

    WMError GetCurrentPiPWindowInfo(std::string& bundleName)
    {
        return WMError::WM_OK;
    }

    WMError GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId)
    {
        return WMError::WM_OK;
    }

    WMError HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken, bool& hasOrNot)
    {
        return WMError::WM_OK;
    }
};
} // namespace OHOS::Rosen
#endif
