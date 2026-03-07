#ifndef ERROR_MANAGER_ANI_UTIL
#define ERROR_MANAGER_ANI_UTIL
 
#include <ani.h>
#include <string>
 
#include "hilog_tag_wrapper.h"
 
namespace OHOS {
namespace AbilityRuntime {
constexpr uint32_t INITITAL_REFCOUNT_ONE = 1;
constexpr uint32_t INITITAL_REFCOUNT_THREE = 3;
constexpr const char* const CLASS_NAME_EVENT_PACKAGE_HOLDER = "@ohos.base.BusinessError";
constexpr const char* RECORD_CLASS_NAME = "std.core.Record";
 
struct GlobalObserverItem {
    ani_ref ref;
    ani_vm* vm;
};

ani_object CreateErrorObject(ani_env *env, ani_string name, ani_string stack, ani_string message,
    bool isError = true, const std::string instanceName = "", ani_int instanceType = 0);
bool IsRefUndefined(ani_env *env, ani_ref ref);
bool IsNull(ani_env *env, ani_ref ref);
ani_vm* GetAniVm(ani_env *env);
ani_env* GetAniEnv(ani_vm *vm);
ani_string StdStringToAniString(ani_env *env, const std::string &str);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // ERROR_MANAGER_ANI_UTIL