#ifndef MOCK_IPC_SKELETON_H
#define MOCK_IPC_SKELETON_H

#include <string>
#include <sys/types.h>
#include "mock_flag.h"

namespace OHOS {
class IPCSkeleton {
public:
    static int32_t GetCallingUid()
    {
        return MockFlag::callingUid;
    }
    static pid_t GetCallingPid()
    {
        return MockFlag::callingPid;
    }
    static std::string ResetCallingIdentity()
    {
        return "";
    }
    static void SetCallingIdentity(const std::string &identity)
    {
        (void)identity;
    }
};
} // namespace OHOS

#endif // MOCK_IPC_SKELETON_H
