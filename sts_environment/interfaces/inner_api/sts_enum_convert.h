/*
 * Copyright (c) 2021-20225 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_STS_ENUM_CONVERT_H
#define OHOS_ABILITY_STS_ENUM_CONVERT_H
#include <array>
#include <string>

namespace OHOS {
namespace AAFwk {

class EnumConvertUtils {
public:
    // enum ColorMode {
    //     COLOR_MODE_NOT_SET = -1,
    //     COLOR_MODE_DARK = 0,
    //     COLOR_MODE_LIGHT = 1
    // }
    static int DarkMode_ConvertSts2Native(const int index);
    static int DarkMode_ConvertNative2Sts(const int nativeValue);

    // enum Direction {
    //     DIRECTION_NOT_SET = -1,
    //     DIRECTION_VERTICAL = 0,
    //     DIRECTION_HORIZONTAL = 1
    //   }
    static int Direction_ConvertSts2Native(const int index);
    static int Direction_ConvertNative2Sts(const int nativeValue);

    static int ScreenDensity_ConvertSts2Native(const int index);
    static int ScreenDensity_ConvertNative2Sts(const int nativeValue);

    //  enum AreaMode {
    //     EL1 = 0,
    //     EL2 = 1,
    //     EL3 = 2,
    //     EL4 = 3,
    //     EL5 = 4
    //   }
    static int AreaMode_ConvertSts2Native(const int index);
    static int AreaMode_ConvertNative2Sts(const int nativeValue);

    static int Flags_ConvertSts2Native(const int index);
    static int Flags_ConvertNative2Sts(const int nativeValue);

    static std::string Action_ConvertSts2Native(const int index);
    static int Action_ConvertNative2Sts(const std::string nativeValue);

    static int WindowMode_ConvertSts2Native(const int index);
    static int WindowMode_ConvertNative2Sts(const int nativeValue);

private:
    //   enum WindowMode {
    //     WINDOW_MODE_UNDEFINED = 0,
    //     WINDOW_MODE_FULLSCREEN = 1,
    //     WINDOW_MODE_SPLIT_PRIMARY = 100,
    //     WINDOW_MODE_SPLIT_SECONDARY = 101,
    //     WINDOW_MODE_FLOATING = 102
    //   }
    static constexpr std::array<int, 5> WindowModeArray = {0, 1, 100, 101, 102};

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
    static constexpr std::array<const char *, 26> ActionArray = {"ohos.want.action.home",
    "ohos.want.action.dial",
    "ohos.want.action.search",
    "ohos.settings.wireless",
    "ohos.settings.manage.applications",
    "ohos.settings.application.details",
    "ohos.want.action.setAlarm",
    "ohos.want.action.showAlarms",
    "ohos.want.action.snoozeAlarm",
    "ohos.want.action.dismissAlarm",
    "ohos.want.action.dismissTimer",
    "ohos.want.action.sendSms",
    "ohos.want.action.choose",
    "ohos.want.action.imageCapture",
    "ohos.want.action.videoCapture",
    "ohos.want.action.select",
    "ohos.want.action.sendData",
    "ohos.want.action.sendMultipleData",
    "ohos.want.action.scanMediaFile",
    "ohos.want.action.viewData",
    "ohos.want.action.editData",
    "ability.want.params.INTENT",
    "ability.want.params.TITLE",
    "ohos.action.fileSelect",
    "ability.params.stream",
    "ohos.account.appAccount.action.oauth"};

    //  enum ScreenDensity {
    //     SCREEN_DENSITY_NOT_SET = 0,
    //     SCREEN_DENSITY_SDPI = 120,
    //     SCREEN_DENSITY_MDPI = 160,
    //     SCREEN_DENSITY_LDPI = 240,
    //     SCREEN_DENSITY_XLDPI = 320,
    //     SCREEN_DENSITY_XXLDPI = 480,
    //     SCREEN_DENSITY_XXXLDPI = 640
    //   }
    static constexpr std::array<int, 7> ScreenDensityArray = {0, 120, 160, 240, 320, 480, 640};

    //     enum Flags {
    //     FLAG_AUTH_READ_URI_PERMISSION = 0x00000001,
    //     FLAG_AUTH_WRITE_URI_PERMISSION = 0x00000002,
    //     FLAG_AUTH_PERSISTABLE_URI_PERMISSION = 0x00000040,
    //     FLAG_INSTALL_ON_DEMAND = 0x00000800,
    //     FLAG_START_WITHOUT_TIPS = 0x40000000
    //   }
    static constexpr std::array<int, 5> FlagsArray = {0x00000001, 0x00000002, 0x00000040, 0x00000800, 0x40000000};
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_STS_ENUM_CONVERT_H
