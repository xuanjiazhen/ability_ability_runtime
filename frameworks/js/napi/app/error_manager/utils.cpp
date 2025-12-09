#include "hilog_tag_wrapper.h"
#include "utils.h"

namespace OHOS {
namespace AbilityRuntime {
ErrorManagerHandleScope::ErrorManagerHandleScope(napi_env env)
{
    env_ = env;
    napi_status scopeStatus = napi_open_handle_scope(env_, &scope_);
    if (scopeStatus != napi_ok || scope_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_open_handle_scope failed");
    } 
}

ErrorManagerHandleScope::~ErrorManagerHandleScope()
{
    napi_close_handle_scope(env_, scope_);
}

}  // namespace AbilityRuntime
}  // namespace OHOS