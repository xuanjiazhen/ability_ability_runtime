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

#ifndef MOCK_COMMON_H
#define MOCK_COMMON_H

#include <string>
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "errors.h"
#include "singleton.h"
#include "wm_common.h"
#include "iremote_broker.h"
#include "want.h"

#define IN_PROCESS_CALL(theCall)                                     \
    ([&]() {                                                         \
        std::string identity = IPCSkeleton::ResetCallingIdentity();  \
        auto retVal = theCall;                                       \
        IPCSkeleton::SetCallingIdentity(identity);                   \
        return retVal;                                               \
    }())

#define IN_PROCESS_CALL_WITHOUT_RET(theCall)                         \
    do {                                                             \
        std::string identity = IPCSkeleton::ResetCallingIdentity();  \
        theCall;                                                     \
        IPCSkeleton::SetCallingIdentity(identity);                   \
    } while (0)

static const std::string KEY_AUTO_FILL_ABILITY = "com.ohos.passwordbox/entry/AutoFillAbility";
static const std::string KEY_SMART_AUTO_FILL_ABILITY = "com.ohos.textautofill/entry/TextAutoFillAbility";

namespace OHOS {
using ErrCode = int;
namespace Rosen {
using DisplayId = uint64_t;
}
const int ERR_NONE = 0;
const int NO_ERROR = 0;
const int INVALID_DATA = 5;
const int ERR_INVALID_STATE = 10;

inline std::u16string Str8ToStr16(const std::string& str)
{
    return u"";
}

inline std::string Str16ToStr8(const std::u16string& str16)
{
    return "";
}

inline void SplitStr(const std::string& str, const std::string& sep, std::vector<std::string>& strs,
    bool canEmpty = false, bool needTrim = true) {}

inline bool StrToInt(const std::string& str, int& value)
{
    return false;
}

enum {
    ERR_NULL_OBJECT = 7,
};

namespace AAFwk {
const int32_t USER_ID_DEFAULT = 100;

enum AbilityWindowConfiguration {
    MULTI_WINDOW_DISPLAY_UNDEFINED = 0,
    MULTI_WINDOW_DISPLAY_FULLSCREEN = 1,
    MULTI_WINDOW_DISPLAY_PRIMARY = 100,
    MULTI_WINDOW_DISPLAY_SECONDARY = 101,
    MULTI_WINDOW_DISPLAY_FLOATING = 102
};

struct ISnapshotHandler {};
struct IMissionListener {};
struct MissionSnapshot {};
struct CallRecord {};
struct SenderInfo {};

class ExtensionConfig : public DelayedSingleton<ExtensionConfig> {
public:
    void LoadExtensionConfiguration() {}
};
} // namespace AAFwk

namespace AppExecFwk {
enum class AbilityState {
    ABILITY_STATE_CREATE = 0,
    ABILITY_STATE_READY,
    ABILITY_STATE_FOREGROUND,
    ABILITY_STATE_FOCUS,
    ABILITY_STATE_BACKGROUND,
    ABILITY_STATE_TERMINATED,
    ABILITY_STATE_END,
    ABILITY_STATE_CONNECTED,
    ABILITY_STATE_DISCONNECTED,
};

constexpr const char* PARAMETER_APP_JUMP_INTERCEPTOR_ENABLE = "is_app_jump_interceptor_enable";

struct PacMap {};
struct IAbilityForegroundStateObserver {};
struct Configuration {};
struct IAbilityFirstFrameStateObserver {};

class IBundleMgr : public IRemoteBroker {
public:
    bool GetBundleNameForUid(int32_t uid, std::string& bundleName)
    {
        return false;
    }
};

enum AppMgrResultCode {
    RESULT_OK = 0,
    ERROR_SERVICE_NOT_READY,
    ERROR_SERVICE_NOT_CONNECTED,
    ERROR_KILL_APPLICATION
};
} // namespace AppExecFwk

namespace AbilityRuntime {
struct IConnectionObserver {};
struct ConnectionData {};
struct DlpConnectionInfo {
    int32_t dlpUid = 0;
    int32_t openedAbilityCount = 0;
};

class ModalSystemAssertUIExtension {
public:
    static ModalSystemAssertUIExtension &GetInstance()
    {
        static ModalSystemAssertUIExtension instance;
        return instance;
    }

    bool CreateModalUIExtension(const AAFwk::Want &want)
    {
        return false;
    }
};
} // namespace AbilityRuntime

namespace Media {
struct PixelMap {};
}

namespace Security {
namespace AccessToken {
typedef unsigned int AccessTokenID;
typedef uint64_t FullTokenID;

typedef enum TypeATokenTypeEnum {
    TOKEN_INVALID = -1,
    TOKEN_HAP = 0,
    TOKEN_NATIVE,
    TOKEN_SHELL,
    TOKEN_TYPE_BUTT,
} ATokenTypeEnum;

typedef enum TypePermissionState {
    PERMISSION_DENIED = -1,
    PERMISSION_GRANTED = 0,
} PermissionState;

struct HapTokenInfo {
    std::string bundleName;
};

struct NativeTokenInfo {
    std::string processName;
};

struct PermissionGrantInfo {
    std::string grantBundleName;
    std::string grantServiceAbilityName;
};
} // namespace AccessToken
} // namespace Security

namespace Rosen {
class FocusChangeInfo {
public:
    sptr<IRemoteObject> abilityToken_;
};

class IFocusChangedListener : public RefBase {};
class IVisibilityChangedListener : public RefBase {};
class WindowVisibilityInfo {
public:
    WindowType windowType_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    WindowVisibilityState visibilityState_ = WINDOW_LAYER_STATE_MAX;
    int32_t pid_ = 0;
};
} // namespace Rosen

namespace MMI {
struct IAnrObserver {};
} // namespace MMI

namespace BackgroundTaskMgr {
class BackgroundTaskSubscriber {};
}  // namespace BackgroundTaskMgr
} // namespace OHOS
#endif
