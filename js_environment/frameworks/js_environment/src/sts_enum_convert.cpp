/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "sts_enum_convert.h"
#include "hilog_tag_wrapper.h"
namespace OHOS {
namespace AAFwk {
// enum ColorMode {
//     COLOR_MODE_NOT_SET = -1,
//     COLOR_MODE_DARK = 0,
//     COLOR_MODE_LIGHT = 1
// }
int EnumConvertUtils::DarkMode_ConvertSts2Native(const int index)
{
    return index - 1; 
}
int EnumConvertUtils::DarkMode_ConvertNative2Sts(const int nativeValue) 
{
    return nativeValue + 1;
}

// enum Direction {
//     DIRECTION_NOT_SET = -1,
//     DIRECTION_VERTICAL = 0,
//     DIRECTION_HORIZONTAL = 1
//   }
int EnumConvertUtils::Direction_ConvertSts2Native(const int index)
{
    return index - 1;
}
int EnumConvertUtils::Direction_ConvertNative2Sts(const int nativeValue)
{
    return nativeValue + 1;
}

//  enum ScreenDensity {
//     SCREEN_DENSITY_NOT_SET = 0,
//     SCREEN_DENSITY_SDPI = 120,
//     SCREEN_DENSITY_MDPI = 160,
//     SCREEN_DENSITY_LDPI = 240,
//     SCREEN_DENSITY_XLDPI = 320,
//     SCREEN_DENSITY_XXLDPI = 480,
//     SCREEN_DENSITY_XXXLDPI = 640
//   }
int EnumConvertUtils::ScreenDensity_ConvertSts2Native(const int index)
{
    if (index < 0 || index >= ScreenDensityArray.size()) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "ScreenDensity_ConvertSts2Native failed index:%{public}d", index);
        return 0;
    }
    return ScreenDensityArray[index];
}
int EnumConvertUtils::ScreenDensity_ConvertNative2Sts(const int nativeValue)
{
    for (int index = 0; index < ScreenDensityArray.size(); index++) {
        if (nativeValue == ScreenDensityArray[index]) {
            return index;
        }
    }
    TAG_LOGE(AAFwkTag::STSRUNTIME, "ScreenDensity_ConvertNative2Sts failed nativeValue:%{public}d", nativeValue);
    return 0;
}

//     enum AreaMode {
//     EL1 = 0,
//     EL2 = 1,
//     EL3 = 2,
//     EL4 = 3,
//     EL5 = 4
//   }
int EnumConvertUtils::AreaMode_ConvertSts2Native(const int index)
{
    return index;
}
int EnumConvertUtils::AreaMode_ConvertNative2Sts(const int nativeValue)
{
    return nativeValue;
}

//     enum Flags {
//     FLAG_AUTH_READ_URI_PERMISSION = 0x00000001,
//     FLAG_AUTH_WRITE_URI_PERMISSION = 0x00000002,
//     FLAG_AUTH_PERSISTABLE_URI_PERMISSION = 0x00000040,
//     FLAG_INSTALL_ON_DEMAND = 0x00000800,
//     FLAG_START_WITHOUT_TIPS = 0x40000000
//   }
int EnumConvertUtils::Flags_ConvertSts2Native(const int index)
{

    if (index < 0 || index >= FlagsArray.size()) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "Flags_ConvertSts2Native failed index:%{public}d", index);
        return 0;
    }
    return FlagsArray[index];
}
int EnumConvertUtils::Flags_ConvertNative2Sts(const int nativeValue)
{
    for (int index = 0; index < FlagsArray.size(); index++) {
        if (nativeValue == FlagsArray[index]) {
            return index;
        }
    }
    TAG_LOGE(AAFwkTag::STSRUNTIME, "Flags_ConvertNative2Sts failed nativeValue:%{public}d", nativeValue);
    return 0;
}
/*
enum Action {
ACTION_HOME = 'ohos.want.action.home',
ACTION_DIAL = 'ohos.want.action.dial',
ACTION_SEARCH = 'ohos.want.action.search',
ACTION_WIRELESS_SETTINGS = 'ohos.settings.wireless',
ACTION_MANAGE_APPLICATIONS_SETTINGS = 'ohos.settings.manage.applications',
ACTION_APPLICATION_DETAILS_SETTINGS = 'ohos.settings.application.details',
ACTION_SET_ALARM = 'ohos.want.action.setAlarm',
ACTION_SHOW_ALARMS = 'ohos.want.action.showAlarms',
ACTION_SNOOZE_ALARM = 'ohos.want.action.snoozeAlarm',
ACTION_DISMISS_ALARM = 'ohos.want.action.dismissAlarm',
ACTION_DISMISS_TIMER = 'ohos.want.action.dismissTimer',
ACTION_SEND_SMS = 'ohos.want.action.sendSms',
ACTION_CHOOSE = 'ohos.want.action.choose',
ACTION_IMAGE_CAPTURE = 'ohos.want.action.imageCapture',
ACTION_VIDEO_CAPTURE = 'ohos.want.action.videoCapture',
ACTION_SELECT = 'ohos.want.action.select',
ACTION_SEND_DATA = 'ohos.want.action.sendData',
ACTION_SEND_MULTIPLE_DATA = 'ohos.want.action.sendMultipleData',
ACTION_SCAN_MEDIA_FILE = 'ohos.want.action.scanMediaFile',
ACTION_VIEW_DATA = 'ohos.want.action.viewData',
ACTION_EDIT_DATA = 'ohos.want.action.editData',
INTENT_PARAMS_INTENT = 'ability.want.params.INTENT',
INTENT_PARAMS_TITLE = 'ability.want.params.TITLE',
ACTION_FILE_SELECT = 'ohos.action.fileSelect',
PARAMS_STREAM = 'ability.params.stream',
ACTION_APP_ACCOUNT_OAUTH = 'ohos.account.appAccount.action.oauth'
}*/
std::string EnumConvertUtils::Action_ConvertSts2Native(const int index)
{
    if (index < 0 || index >= ActionArray.size()) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "Action_ConvertSts2Native failed index:%{public}d", index);
        return 0;
    }
    return ActionArray[index];
}
int EnumConvertUtils::Action_ConvertNative2Sts(const std::string nativeValue)
{
    for (int index = 0; index < ActionArray.size(); index++) {
        if (nativeValue == ActionArray[index]) {
            return index;
        }
    }
    TAG_LOGE(AAFwkTag::STSRUNTIME, "Action_ConvertSts2Native failed nativeValue:%{public}s", nativeValue.c_str());
    return 0;
}

//   enum WindowMode {
//     WINDOW_MODE_UNDEFINED = 0,
//     WINDOW_MODE_FULLSCREEN = 1,
//     WINDOW_MODE_SPLIT_PRIMARY = 100,
//     WINDOW_MODE_SPLIT_SECONDARY = 101,
//     WINDOW_MODE_FLOATING = 102
//   }
int EnumConvertUtils::WindowMode_ConvertSts2Native(const int index)
{

    if (index < 0 || index >= WindowModeArray.size()) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "WindowMode_ConvertSts2Native failed index:%{public}d", index);
        return 0;
    }
    return WindowModeArray[index];
}
int EnumConvertUtils::WindowMode_ConvertNative2Sts(const int nativeValue)
{
    for (int index = 0; index < WindowModeArray.size(); index++) {
        if (nativeValue == WindowModeArray[index]) {
            return index;
        }
    }
    TAG_LOGE(AAFwkTag::STSRUNTIME, "WindowMode_ConvertNative2Sts failed nativeValue:%{public}d", nativeValue);
    return 0;
}
}  // namespace AAFwk
}  // namespace OHOS
