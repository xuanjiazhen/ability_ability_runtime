/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

import StartupConfig from '@ohos.app.appstartup.StartupConfig';
import common from '@ohos.app.ability.common';
import { BusinessError, AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';

export default namespace startupManager {
    loadLibraryWithPermissionCheck("startup_manager_ani_kit.z", "@ohos.app.appstartup.startupManager");
    
    export native function nativeRun(startupTasks: Array<string>, context: common.AbilityStageContext,
        config: StartupConfig, callback: AsyncCallbackWrapper<void>): void;
    export native function nativeRemoveAllStartupTaskResults():
}