#ifndef MOCK_APP_MGR_CLIENT_H
#define MOCK_APP_MGR_CLIENT_H

#include "running_process_info.h"
#include "mock_flag.h"

namespace OHOS {
namespace AppExecFwk {

class AppMgrClient {
    DECLARE_DELAYED_SINGLETON(AppMgrClient);
public:
    int32_t GetRunningProcessInfoByPid(const pid_t pid, RunningProcessInfo &info)
    {
        if (MockFlag::getRunningProcessInfoRet != 0) {
            return MockFlag::getRunningProcessInfoRet;
        }
        info.state_ = static_cast<AppProcessState>(MockFlag::processState);
        info.isPreForeground = MockFlag::isPreForeground;
        return 0;
    }
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_APP_MGR_CLIENT_H
