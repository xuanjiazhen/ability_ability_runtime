#ifndef MOCK_RUNNING_PROCESS_INFO_H
#define MOCK_RUNNING_PROCESS_INFO_H

#include <cstdint>

namespace OHOS {
namespace AppExecFwk {

enum class AppProcessState {
    APP_STATE_BEGIN = 0,
    APP_STATE_READY = 1,
    APP_STATE_FOREGROUND = 2,
    APP_STATE_BACKGROUND = 4,
    APP_STATE_END
};

struct RunningProcessInfo {
    AppProcessState state_ = AppProcessState::APP_STATE_FOREGROUND;
    bool isPreForeground = false;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_RUNNING_PROCESS_INFO_H
