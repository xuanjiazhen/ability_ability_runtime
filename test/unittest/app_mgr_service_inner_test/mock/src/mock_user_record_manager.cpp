#include "user_record_manager.h"
#include "mock_my_flag.h"

#include <mutex>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

bool UserRecordManager::IsLogoutUser(int32_t userId)
{
    return AAFwk::MyFlag::userId_;
}
} // namespace AppExecFwk
} // namespace OHOS