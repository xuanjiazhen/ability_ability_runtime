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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_ERROR_MSG_H
#define OHOS_ABILITY_RUNTIME_ABILITY_ERROR_MSG_H

namespace OHOS {
namespace AbilityRuntime {
inline constexpr char ERR_MSG_CALL_CONNECTION_NOT_READY[] =
    "Internal error. The caller connection is not ready. Call startAbilityByCall again after the target ability is "
    "ready.";
inline constexpr char ERR_MSG_SERVICE_CONNECTION_NOT_FOUND[] =
    "Internal error. The service connection does not exist. Use a connection ID returned by "
    "connectServiceExtensionAbility.";
inline constexpr char ERR_MSG_RESTORE_WINDOW_STAGE_FAILED[] =
    "Internal error. The window stage could not be restored. Call restoreWindowStage with a valid content storage "
    "object.";
inline constexpr char ERR_MSG_START_ABILITY_FAILED[] =
    "Internal error. The ability could not be started. Check whether starting abilities is currently allowed and try "
    "again.";
inline constexpr char ERR_MSG_START_ABILITY_AS_CALLER_FAILED[] =
    "Internal error. The ability could not be started as caller. Verify the caller ability is valid and foreground, "
    "then try again.";
inline constexpr char ERR_MSG_START_SELF_FAILED[] =
    "Internal error. The ability could not start itself. Verify the caller process is foreground and starting abilities "
    "is allowed.";
inline constexpr char ERR_MSG_MODAL_UI_EXTENSION_FAILED[] =
    "Internal error. The modal UI extension could not be created. Ensure the caller is the focused foreground "
    "UIAbility and try again.";
inline constexpr char ERR_MSG_SKIP_SCENARIOS_FAILED[] =
    "Internal error. The skip scenario setting could not be applied. Verify the scenario value and call this API while "
    "the ability is active.";
inline constexpr char ERR_MSG_START_UI_SERVICE_EXT_FAILED[] =
    "Internal error. The UI service extension could not be started. Check the extension configuration and try again.";
inline constexpr char ERR_MSG_CONNECT_UI_SERVICE_EXT_FAILED[] =
    "Internal error. The UI service extension could not be connected. Check the extension configuration and try again.";
inline constexpr char ERR_MSG_START_EXTENSION_FAILED[] =
    "Internal error. The extension ability could not be started. Check the Want parameters and extension "
    "configuration.";
inline constexpr char ERR_MSG_STOP_EXTENSION_FAILED[] =
    "Internal error. The extension ability could not be stopped. Check the Want parameters and extension status.";
inline constexpr char ERR_MSG_TERMINATE_SELF_FAILED[] =
    "Internal error. Failed to terminate the ability. Ensure the ability is active and try again.";
inline constexpr char ERR_MSG_TERMINATE_SELF_WITH_RESULT_FAILED[] =
    "Internal error. Failed to terminate the ability with result. Ensure the ability is active and try again.";
inline constexpr char ERR_MSG_BACK_TO_CALLER_FAILED[] =
    "Internal error. Failed to return the result to the caller ability. Verify the request code is valid.";
inline constexpr char ERR_MSG_START_ABILITY_FOR_RESULT_FAILED[] =
    "Internal error. The ability for result could not be started. Check the Want parameters and try again.";
inline constexpr char ERR_MSG_REPORT_DRAWN_FAILED[] =
    "Internal error. Failed to report drawn completed. Ensure the ability window is ready.";
inline constexpr char ERR_MSG_SET_MISSION_CONTINUE_STATE_FAILED[] =
    "Internal error. Failed to set the mission continue state. Call this API while the ability is in the foreground.";
inline constexpr char ERR_MSG_SET_MISSION_LABEL_FAILED[] =
    "Internal error. Failed to set the mission label. Call this API while the ability is in the foreground.";
inline constexpr char ERR_MSG_SET_MISSION_ICON_FAILED[] =
    "Internal error. Failed to set the mission icon. Provide a valid PixelMap and call this API while the ability is "
    "in the foreground.";
inline constexpr char ERR_MSG_START_ABILITY_BY_TYPE_FAILED[] =
    "Internal error. The ability could not be started by type. Check the type and parameters, then try again.";
inline constexpr char ERR_MSG_CHANGE_ABILITY_VISIBILITY_FAILED[] =
    "Internal error. Failed to change the ability visibility. Ensure the ability is active and try again.";
inline constexpr char ERR_MSG_MOVE_ABILITY_TO_BACKGROUND_FAILED[] =
    "Internal error. Failed to move the ability to background. Ensure the ability is in the foreground.";
inline constexpr char ERR_MSG_OPEN_LINK_FAILED[] =
    "Internal error. Failed to open the link. Check the URI and link options, then try again.";
inline constexpr char ERR_MSG_REQUEST_DIALOG_SERVICE_FAILED[] =
    "Internal error. Failed to request the dialog service. Check the Want parameters and try again.";
inline constexpr char ERR_MSG_REVOKE_DELEGATOR_FAILED[] =
    "Internal error. Failed to revoke the delegator. Call this API while the ability is active.";
inline constexpr char ERR_MSG_RESTART_APP_FAILED[] =
    "Internal error. Failed to restart the app. Check the Want parameters and try again.";
inline constexpr char ERR_MSG_SET_MISSION_WINDOW_ICON_FAILED[] =
    "Internal error. Failed to set the mission window icon. Provide a valid PixelMap and call this API while the "
    "ability is active.";
inline constexpr char ERR_MSG_SET_ABILITY_INSTANCE_INFO_FAILED[] =
    "Internal error. Failed to set the ability instance info. Provide valid parameters and call this API while the "
    "ability is active.";
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABILITY_ERROR_MSG_H
