/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mock_ani_env.h"

#include <algorithm>

namespace OHOS {
namespace EtsEnv {
namespace {
constexpr uintptr_t MOCK_REF = 0x11;
constexpr uintptr_t MOCK_ERROR = 0x21;
constexpr uintptr_t MOCK_ERROR_TYPE = 0x22;
constexpr uintptr_t MOCK_NAME_GETTER = 0x23;
constexpr uintptr_t MOCK_MESSAGE_GETTER = 0x24;
constexpr uintptr_t MOCK_STACK_GETTER = 0x25;
constexpr uintptr_t MOCK_NAME_STRING = 0x26;
constexpr uintptr_t MOCK_MESSAGE_STRING = 0x27;
constexpr uintptr_t MOCK_STACK_STRING = 0x28;

const std::string &GetMockStringForHandle(ani_string string, const MockAniEnv::State &state)
{
    static const std::string EMPTY_STRING;
    auto handle = reinterpret_cast<uintptr_t>(string);
    if (handle == MOCK_NAME_STRING) {
        return state.errorName;
    }
    if (handle == MOCK_MESSAGE_STRING) {
        return state.errorMessage;
    }
    if (handle == MOCK_STACK_STRING) {
        return state.errorStack;
    }
    return EMPTY_STRING;
}
} // namespace

MockAniEnv::State MockAniEnv::state_;

MockAniEnv::MockAniEnv()
{
    api_.GetUndefined = &MockAniEnv::GetUndefined;
    api_.Array_New = &MockAniEnv::ArrayNew;
    api_.String_NewUTF8 = &MockAniEnv::StringNewUtf8;
    api_.Array_Set = &MockAniEnv::ArraySet;
    api_.Object_CallMethodByName_Void_V = &MockAniEnv::CallMethodVoid;

    api_.FindClass = &MockAniEnv::FindClass;
    api_.Class_FindMethod = &MockAniEnv::ClassFindMethod;
    api_.Class_FindField = &MockAniEnv::ClassFindField;
    api_.Object_New_V = &MockAniEnv::ObjectNew;
    api_.Object_GetType = &MockAniEnv::ObjectGetType;
    api_.Class_FindGetter = &MockAniEnv::ClassFindGetter;
    api_.Object_CallMethod_Ref_V = &MockAniEnv::ObjectCallMethodRef;
    api_.Object_GetField_Long = &MockAniEnv::ObjectGetFieldLong;
    api_.Object_SetField_Long = &MockAniEnv::ObjectSetFieldLong;
    api_.GlobalReference_Create = &MockAniEnv::GlobalReferenceCreate;
    api_.GlobalReference_Delete = &MockAniEnv::GlobalReferenceDelete;
    api_.Class_CallStaticMethodByName_Void_V = &MockAniEnv::ClassCallStaticMethodByNameVoid;
    api_.ExistUnhandledError = &MockAniEnv::ExistUnhandledError;
    api_.GetUnhandledError = &MockAniEnv::GetUnhandledError;
    api_.ResetError = &MockAniEnv::ResetError;
    api_.String_GetUTF8Size = &MockAniEnv::StringGetUtf8Size;
    api_.String_GetUTF8SubString = &MockAniEnv::StringGetUtf8SubString;

    env_.c_api = &api_;

    MockAniEnv::state_.getUndefinedStatus = ANI_OK;
    MockAniEnv::state_.arrayNewStatus = ANI_OK;
    MockAniEnv::state_.stringNewStatus = ANI_OK;
    MockAniEnv::state_.arraySetStatus = ANI_OK;
    MockAniEnv::state_.callMethodStatus = ANI_OK;
    MockAniEnv::state_.findClassStatus = ANI_OK;
    MockAniEnv::state_.classFindMethodStatus = ANI_OK;
    MockAniEnv::state_.classFindFieldStatus = ANI_OK;
    MockAniEnv::state_.objectNewStatus = ANI_OK;
    MockAniEnv::state_.objectGetTypeStatus = ANI_OK;
    MockAniEnv::state_.classFindGetterStatus = ANI_OK;
    MockAniEnv::state_.objectCallMethodRefStatus = ANI_OK;
    MockAniEnv::state_.objectGetFieldLongStatus = ANI_OK;
    MockAniEnv::state_.objectSetFieldLongStatus = ANI_OK;
    MockAniEnv::state_.globalRefCreateStatus = ANI_OK;
    MockAniEnv::state_.globalRefDeleteStatus = ANI_OK;
    MockAniEnv::state_.arrayNewRefStatus = ANI_OK;
    MockAniEnv::state_.arraySetRefStatus = ANI_OK;
    MockAniEnv::state_.classCallStaticMethodStatus = ANI_OK;
    MockAniEnv::state_.existUnhandledErrorStatus = ANI_OK;
    MockAniEnv::state_.getUnhandledErrorStatus = ANI_OK;
    MockAniEnv::state_.resetErrorStatus = ANI_OK;
    MockAniEnv::state_.stringGetUtf8SizeStatus = ANI_OK;
    MockAniEnv::state_.stringGetUtf8SubStringStatus = ANI_OK;
    MockAniEnv::state_.unhandledErrorExists = ANI_FALSE;
    MockAniEnv::state_.errorName.clear();
    MockAniEnv::state_.errorMessage.clear();
    MockAniEnv::state_.errorStack.clear();
    MockAniEnv::state_.callMethodStatusSequence.clear();
    MockAniEnv::state_.callMethodStatusIndex = 0;
}

ani_env *MockAniEnv::GetEnv()
{
    return &env_;
}

MockAniEnv::State &MockAniEnv::GetState()
{
    return MockAniEnv::state_;
}

ani_status MockAniEnv::GetUndefined(ani_env *env, ani_ref *result)
{
    if (MockAniEnv::state_.getUndefinedStatus != ANI_OK) {
        return MockAniEnv::state_.getUndefinedStatus;
    }
    *result = reinterpret_cast<ani_ref>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::ArrayNew(ani_env *env, ani_size length, ani_ref initValue, ani_array *result)
{
    if (MockAniEnv::state_.arrayNewStatus != ANI_OK) {
        return MockAniEnv::state_.arrayNewStatus;
    }
    *result = reinterpret_cast<ani_array>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::ArrayNewRef(ani_env *env, ani_type type, ani_size length,
    ani_ref initValue, ani_array *result)
{
    if (MockAniEnv::state_.arrayNewRefStatus != ANI_OK) {
        return MockAniEnv::state_.arrayNewRefStatus;
    }
    *result = reinterpret_cast<ani_array>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::StringNewUtf8(ani_env *env, const char *utf8, ani_size len, ani_string *result)
{
    if (MockAniEnv::state_.stringNewStatus != ANI_OK) {
        return MockAniEnv::state_.stringNewStatus;
    }
    *result = reinterpret_cast<ani_string>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::ArraySet(ani_env *env, ani_array array, ani_size index, ani_ref value)
{
    return MockAniEnv::state_.arraySetStatus;
}

ani_status MockAniEnv::ArraySetRef(ani_env *env, ani_array array, ani_size index, ani_ref value)
{
    return MockAniEnv::state_.arraySetRefStatus;
}

ani_status MockAniEnv::CallMethodVoid(ani_env *env, ani_object obj, const char *name, const char *sig, va_list args)
{
    if (MockAniEnv::state_.callMethodStatusIndex < MockAniEnv::state_.callMethodStatusSequence.size()) {
        return MockAniEnv::state_.callMethodStatusSequence[MockAniEnv::state_.callMethodStatusIndex++];
    }
    return MockAniEnv::state_.callMethodStatus;
}

ani_status MockAniEnv::ClassCallStaticMethodByNameVoid(ani_env *env, ani_class cls, const char *name,
    const char *sig, va_list args)
{
    return MockAniEnv::state_.classCallStaticMethodStatus;
}

ani_status MockAniEnv::FindClass(ani_env *env, const char *name, ani_class *cls)
{
    if (MockAniEnv::state_.findClassStatus != ANI_OK) {
        return MockAniEnv::state_.findClassStatus;
    }
    *cls = reinterpret_cast<ani_class>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::ClassFindMethod(ani_env *env, ani_class cls, const char *name, const char *sig,
    ani_method *method)
{
    if (MockAniEnv::state_.classFindMethodStatus != ANI_OK) {
        return MockAniEnv::state_.classFindMethodStatus;
    }
    *method = reinterpret_cast<ani_method>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::ClassFindField(ani_env *env, ani_class cls, const char *name, ani_field *field)
{
    if (MockAniEnv::state_.classFindFieldStatus != ANI_OK) {
        return MockAniEnv::state_.classFindFieldStatus;
    }
    *field = reinterpret_cast<ani_field>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::ObjectNew(ani_env *env, ani_class cls, ani_method method, ani_object *result, va_list args)
{
    if (MockAniEnv::state_.objectNewStatus != ANI_OK) {
        return MockAniEnv::state_.objectNewStatus;
    }
    *result = reinterpret_cast<ani_object>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::ObjectGetType(ani_env *env, ani_object obj, ani_type *result)
{
    if (MockAniEnv::state_.objectGetTypeStatus != ANI_OK) {
        return MockAniEnv::state_.objectGetTypeStatus;
    }
    *result = reinterpret_cast<ani_type>(MOCK_ERROR_TYPE);
    return ANI_OK;
}

ani_status MockAniEnv::ClassFindGetter(ani_env *env, ani_class cls, const char *name, ani_method *result)
{
    if (MockAniEnv::state_.classFindGetterStatus != ANI_OK) {
        return MockAniEnv::state_.classFindGetterStatus;
    }
    std::string property = name == nullptr ? "" : name;
    if (property == "name") {
        *result = reinterpret_cast<ani_method>(MOCK_NAME_GETTER);
        return ANI_OK;
    }
    if (property == "message") {
        *result = reinterpret_cast<ani_method>(MOCK_MESSAGE_GETTER);
        return ANI_OK;
    }
    if (property == "stack") {
        *result = reinterpret_cast<ani_method>(MOCK_STACK_GETTER);
        return ANI_OK;
    }
    return ANI_NOT_FOUND;
}

ani_status MockAniEnv::ObjectCallMethodRef(ani_env *env, ani_object obj, ani_method method, ani_ref *result,
    va_list args)
{
    if (MockAniEnv::state_.objectCallMethodRefStatus != ANI_OK) {
        return MockAniEnv::state_.objectCallMethodRefStatus;
    }
    auto methodHandle = reinterpret_cast<uintptr_t>(method);
    if (methodHandle == MOCK_NAME_GETTER) {
        *result = reinterpret_cast<ani_ref>(MOCK_NAME_STRING);
        return ANI_OK;
    }
    if (methodHandle == MOCK_MESSAGE_GETTER) {
        *result = reinterpret_cast<ani_ref>(MOCK_MESSAGE_STRING);
        return ANI_OK;
    }
    if (methodHandle == MOCK_STACK_GETTER) {
        *result = reinterpret_cast<ani_ref>(MOCK_STACK_STRING);
        return ANI_OK;
    }
    return ANI_NOT_FOUND;
}

ani_status MockAniEnv::StringGetUtf8Size(ani_env *env, ani_string string, ani_size *result)
{
    if (MockAniEnv::state_.stringGetUtf8SizeStatus != ANI_OK) {
        return MockAniEnv::state_.stringGetUtf8SizeStatus;
    }
    *result = GetMockStringForHandle(string, MockAniEnv::state_).size();
    return ANI_OK;
}

ani_status MockAniEnv::StringGetUtf8SubString(ani_env *env, ani_string string, ani_size substrOffset,
    ani_size substrSize, char *buffer, ani_size bufferSize, ani_size *result)
{
    if (MockAniEnv::state_.stringGetUtf8SubStringStatus != ANI_OK) {
        return MockAniEnv::state_.stringGetUtf8SubStringStatus;
    }
    const auto &value = GetMockStringForHandle(string, MockAniEnv::state_);
    if (substrOffset > value.size()) {
        return ANI_OUT_OF_RANGE;
    }
    auto copyLength = std::min(static_cast<size_t>(substrSize), value.size() - substrOffset);
    if (bufferSize <= copyLength) {
        return ANI_BUFFER_TO_SMALL;
    }
    std::copy_n(value.data() + substrOffset, copyLength, buffer);
    buffer[copyLength] = '\0';
    *result = copyLength;
    return ANI_OK;
}

ani_status MockAniEnv::ObjectGetFieldLong(ani_env *env, ani_object obj, ani_field field, ani_long *result)
{
    if (MockAniEnv::state_.objectGetFieldLongStatus != ANI_OK) {
        return MockAniEnv::state_.objectGetFieldLongStatus;
    }
    *result = static_cast<ani_long>(0);
    return ANI_OK;
}

ani_status MockAniEnv::ObjectSetFieldLong(ani_env *env, ani_object obj, ani_field field, ani_long value)
{
    return MockAniEnv::state_.objectSetFieldLongStatus;
}

ani_status MockAniEnv::GlobalReferenceCreate(ani_env *env, ani_ref ref, ani_ref *result)
{
    if (MockAniEnv::state_.globalRefCreateStatus != ANI_OK) {
        return MockAniEnv::state_.globalRefCreateStatus;
    }
    *result = reinterpret_cast<ani_object>(MOCK_REF);
    return ANI_OK;
}

ani_status MockAniEnv::GlobalReferenceDelete(ani_env *env, ani_ref ref)
{
    return MockAniEnv::state_.globalRefDeleteStatus;
}

ani_status MockAniEnv::ExistUnhandledError(ani_env *env, ani_boolean *result)
{
    if (MockAniEnv::state_.existUnhandledErrorStatus != ANI_OK) {
        return MockAniEnv::state_.existUnhandledErrorStatus;
    }
    *result = MockAniEnv::state_.unhandledErrorExists;
    return ANI_OK;
}

ani_status MockAniEnv::GetUnhandledError(ani_env *env, ani_error *result)
{
    if (MockAniEnv::state_.getUnhandledErrorStatus != ANI_OK) {
        return MockAniEnv::state_.getUnhandledErrorStatus;
    }
    *result = reinterpret_cast<ani_error>(MOCK_ERROR);
    return ANI_OK;
}

ani_status MockAniEnv::ResetError(ani_env *env)
{
    if (MockAniEnv::state_.resetErrorStatus != ANI_OK) {
        return MockAniEnv::state_.resetErrorStatus;
    }
    MockAniEnv::state_.unhandledErrorExists = ANI_FALSE;
    return ANI_OK;
}
} // namespace EtsEnv
} // namespace OHOS
