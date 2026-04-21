#ifndef MOCK_BUNDLE_MGR_HELPER_H
#define MOCK_BUNDLE_MGR_HELPER_H

#include <string>
#include <memory>
#include "mock_flag.h"
#include "ability_record/ability_request.h"

namespace OHOS {
namespace AppExecFwk {

class BundleMgrHelper {
    DECLARE_DELAYED_SINGLETON(BundleMgrHelper)
public:
    int32_t GetNameAndIndexForUid(int32_t uid, std::string &bundleName, int32_t &appIndex)
    {
        if (MockFlag::getNameAndIndexRet != 0) {
            return MockFlag::getNameAndIndexRet;
        }
        bundleName = "com.caller.bundle";
        appIndex = 0;
        return 0;
    }
    bool GetApplicationInfoWithAppIndex(const std::string &appName, int32_t appIndex,
        int32_t userId, ApplicationInfo &appInfo)
    {
        if (!MockFlag::getApplicationInfoRet) {
            return false;
        }
        appInfo.appDistributionType = "debug";
        appInfo.uid = 1000;
        return true;
    }
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_BUNDLE_MGR_HELPER_H
