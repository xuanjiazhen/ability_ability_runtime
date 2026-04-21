#ifndef MOCK_PARAMETERS_H
#define MOCK_PARAMETERS_H

#include <string>
#include "mock_flag.h"

namespace OHOS {
namespace system {
inline bool GetBoolParameter(const std::string &key, bool defaultValue)
{
    (void)key;
    (void)defaultValue;
    return MockFlag::isDeveloperMode;
}
} // namespace system
} // namespace OHOS

#endif // MOCK_PARAMETERS_H
