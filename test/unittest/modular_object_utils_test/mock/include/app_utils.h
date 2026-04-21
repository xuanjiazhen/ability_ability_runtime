#ifndef MOCK_APP_UTILS_H
#define MOCK_APP_UTILS_H

#include "mock_flag.h"

namespace OHOS {
namespace AAFwk {
class AppUtils {
public:
    static AppUtils &GetInstance()
    {
        static AppUtils instance;
        return instance;
    }
    bool IsSupportModularObjectExtension()
    {
        return MockFlag::isSupportModularObjectExtension;
    }
};
} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_APP_UTILS_H
