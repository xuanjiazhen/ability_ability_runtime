#ifndef MOCK_OS_ACCOUNT_MANAGER_WRAPPER_H
#define MOCK_OS_ACCOUNT_MANAGER_WRAPPER_H

#include <cstdint>
#include "mock_flag.h"

namespace OHOS {
namespace AppExecFwk {

class OsAccountManagerWrapper {
    DECLARE_DELAYED_SINGLETON(OsAccountManagerWrapper)
public:
    int32_t GetOsAccountLocalIdFromUid(int32_t uid, int32_t &id)
    {
        if (MockFlag::getOsAccountRet != 0) {
            return MockFlag::getOsAccountRet;
        }
        id = 100;
        return 0;
    }
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_OS_ACCOUNT_MANAGER_WRAPPER_H
