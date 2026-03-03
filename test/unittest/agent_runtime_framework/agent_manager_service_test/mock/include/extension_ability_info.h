/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_ABILITY_INFO_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_ABILITY_INFO_H

#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
enum class ExtensionAbilityType {
    UNSPECIFIED = 0,
    FORM = 1,
    WORK_SCHEDULER = 2,
    DATA_SHARE = 3,
    DATA_SHARE_PROXY = 4,
    AUDIO_PLAYBACK = 5,
    AUDIO_RECORD = 6,
    WALLPAPER = 7,
    INPUT_METHOD = 8,
    SERVICE = 9,
    LAUNCHER = 10,
    UNSPECIFIED_DATA = 11,
    FILE_SHARE = 12,
    THUMBNAIL = 13,
    PREVIEW = 14,
    PRINT = 15,
    PUSH = 16,
    WEB = 17,
    WINDOW = 18,
    ACCOUNT = 19,
    ENTERPRISE = 20,
    GEOSPATIAL = 21,
    CALL = 22,
    SCANNER = 23,
    DLNA = 24,
    RCE = 25,
    STATIC_SUBSCRIBER = 26,
    STATIC_PROVIDER = 27,
    CLOUD_SYNC = 28,
    ACCESSIBILITY = 29,
    ACTION = 30,
    AGENT = 31,
    CONTINUOUS_TASK = 32,
    EMBEDDED_UI = 33,
    EMBEDDED_UI_JS = 34,
    ASSISTANT = 35,
    NAVIGATION_TARGET = 36,
    IMMERSIVE = 37,
    IMMERSIVE_AI = 38,
    IMMERSIVE_AI_JS = 39,
    SHORTCUT = 40,
    SHORTCUT_JS = 41,
    BUSINESS = 42,
    BUSINESS_JS = 43,
    SHARE = 44,
    SHARE_JS = 45,
    SECURITY_GUARD = 46,
    SMART_REMINDER = 47,
    SMART_REMINDER_JS = 48,
    WALLPAPER_LIVE = 49,
    WALLPAPER_LIVE_JS = 50,
    SCREEN_CAPTURE = 51,
    SCREEN_CAPTURE_JS = 52,
    UI_EXTENSION = 53,
    UI_EXTENSION_JS = 54,
    PRINT_SERVICE = 55,
    PRINT_SERVICE_JS = 56,
    DATA_URI = 57,
    DATA_URI_JS = 58,
    DATA_URI_PROXY = 59,
    DATA_URI_PROXY_JS = 60,
    AUTO_STARTUP = 61,
    AUTO_STARTUP_JS = 62,
    SYSTEM_UI = 63,
    SYSTEM_UI_JS = 64,
    VIRTUAL_KEYBOARD = 65,
    VIRTUAL_KEYBOARD_JS = 66,
    VOICE_INTERACTION = 67,
    VOICE_INTERACTION_JS = 68,
    SYSTEM_SHARE = 69,
    SYSTEM_SHARE_JS = 70,
    EMBEDDED_BROWSER = 71,
    EMBEDDED_BROWSER_JS = 72,
    WALLPAPER_DOWNLOAD = 73,
    WALLPAPER_DOWNLOAD_JS = 74,
    DOWNLOAD = 75,
    DOWNLOAD_JS = 76,
    BACKUP = 77,
    BACKUP_JS = 78,
    APP_LINKING = 79,
    APP_LINKING_JS = 80,
    MULTI_WINDOW = 81,
    MULTI_WINDOW_JS = 82,
    MINING_WINDOW = 83,
    MINING_WINDOW_JS = 84,
    DRAG_AND_DROP = 85,
    DRAG_AND_DROP_JS = 86,
    TASK_PIN = 87,
    TASK_PIN_JS = 88,
    DYNAMIC_ISLAND = 89,
    DYNAMIC_ISLAND_JS = 90,
    ISLAND_HOST = 91,
    ISLAND_HOST_JS = 92,
    SHARE_PANEL = 93,
    SHARE_PANEL_JS = 94,
    CROP = 95,
    CROP_JS = 96,
    SYSTEM_PICKER = 97,
    SYSTEM_PICKER_JS = 98,
    DOCUMENT_PICKER = 99,
    DOCUMENT_PICKER_JS = 100,
    PHOTO_PICKER = 101,
    PHOTO_PICKER_JS = 102,
    VIDEO_PICKER = 103,
    VIDEO_PICKER_JS = 104,
    FILE_PICKER = 105,
    FILE_PICKER_JS = 106,
    MULTI_PICKER = 107,
    MULTI_PICKER_JS = 108,
    TYPE_MAX = 109,
};

struct ExtensionAbilityInfo {
    std::string bundleName;
    std::string moduleName;
    std::string name;
    std::string description;
    std::string icon;
    std::string label;
    ExtensionAbilityType type = ExtensionAbilityType::UNSPECIFIED;
    std::string permissions;
    std::string readPermission;
    std::string writePermission;
    std::string uri;
    std::string extensionAbilityType;
    std::string metadata;
    std::string srcEntrance;
    bool visible = false;
    int32_t id = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_EXTENSION_ABILITY_INFO_H
