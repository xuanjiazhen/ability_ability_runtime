#ifndef OHOS_ABILITY_RUNTIME_DMS_INTENT_CALLER_INFO_H
#define OHOS_ABILITY_RUNTIME_DMS_INTENT_CALLER_INFO_H

#include <cstdint>

namespace OHOS {
namespace AAFwk {
struct IntentCallerInfo {
    int32_t callerUid = 0;
    uint64_t requestCode = 0;
    uint32_t accessToken = 0;
    uint32_t specifyTokenId = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DMS_INTENT_CALLER_INFO_H
