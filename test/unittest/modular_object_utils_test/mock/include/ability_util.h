#ifndef MOCK_ABILITY_UTIL_H
#define MOCK_ABILITY_UTIL_H

#include "ipc_skeleton.h"

#define IN_PROCESS_CALL(theCall)                                 \
    ([&]() {                                                     \
        std::string identity = OHOS::IPCSkeleton::ResetCallingIdentity(); \
        auto retVal = theCall;                                   \
        OHOS::IPCSkeleton::SetCallingIdentity(identity);        \
        return retVal;                                           \
    }())

#define CHECK_POINTER_AND_RETURN(object, value) \
    if (!object) {                              \
        return value;                           \
    }

#endif // MOCK_ABILITY_UTIL_H
