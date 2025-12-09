#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
class ErrorManagerHandleScope final {
public:
    explicit ErrorManagerHandleScope(napi_env env);
    ~ErrorManagerHandleScope();

    ErrorManagerHandleScope(const ErrorManagerHandleScope&) = delete;
    ErrorManagerHandleScope(ErrorManagerHandleScope&&) = delete;
    ErrorManagerHandleScope& operator=(const ErrorManagerHandleScope&) = delete;
    ErrorManagerHandleScope& operator=(ErrorManagerHandleScope&&) = delete;
private:
    napi_handle_scope scope_ = nullptr;
    napi_env env_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS