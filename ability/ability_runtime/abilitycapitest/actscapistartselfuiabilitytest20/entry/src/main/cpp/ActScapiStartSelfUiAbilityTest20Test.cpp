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

#include "AbilityKit/native_child_process.h"
#include "napi/native_api.h"
#include <cstdint>
#include <js_native_api_types.h>
#include <napi/native_api.h>
#include "AbilityKit/ability_runtime/application_context.h"

namespace {
static const uint32_t MAX_NAME_LENGTH = 1024;
}

static napi_value DestroyChildProcessConfigsParamInvalid(napi_env env, napi_callback_info info)
{
    Ability_ChildProcessConfigs *configs = nullptr;
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_DestroyChildProcessConfigs(configs);
    if (ret == NCP_ERR_INVALID_PARAM)
        {
        // 错误处理
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
        }
    return nullptr;
}

static napi_value DestroyChildProcessConfigsNoError(napi_env env, napi_callback_info info)
{
    Ability_ChildProcessConfigs *configs = OH_Ability_CreateChildProcessConfigs();
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_DestroyChildProcessConfigs(configs);
    if (ret == NCP_NO_ERROR) {
        // 错误处理
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
    }
    return nullptr;
}

static napi_value SetIsolationModeConfigsInvalidParam(napi_env env, napi_callback_info info)
{
    Ability_ChildProcessConfigs *configs = nullptr;
    Ability_NativeChildProcess_ErrCode ret =
        OH_Ability_ChildProcessConfigs_SetIsolationMode(configs, NCP_ISOLATION_MODE_ISOLATED);
    if (ret == NCP_ERR_INVALID_PARAM) {
        // 错误处理
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
    }
    return nullptr;
}

static napi_value SetIsolationModeConfigsNoError(napi_env env, napi_callback_info info)
{
    Ability_ChildProcessConfigs *configs = OH_Ability_CreateChildProcessConfigs();
    Ability_NativeChildProcess_ErrCode ret =
        OH_Ability_ChildProcessConfigs_SetIsolationMode(configs, NCP_ISOLATION_MODE_ISOLATED);
    if (ret == NCP_NO_ERROR) {
        // 错误处理
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
    }
    return nullptr;
}

static napi_value SetProcessNameConfigsInvalidParam(napi_env env, napi_callback_info info)
{
    Ability_ChildProcessConfigs *configs;
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_ChildProcessConfigs_SetProcessName(configs, nullptr);
    if (ret == NCP_ERR_INVALID_PARAM) {
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
    }
    return nullptr;
}

static napi_value SetProcessNameConfigsNoError(napi_env env, napi_callback_info info)
{
    Ability_ChildProcessConfigs *configs = OH_Ability_CreateChildProcessConfigs();
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_ChildProcessConfigs_SetProcessName(configs, "abc_123");
    if (ret == NCP_NO_ERROR) {
        // 错误处理
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
    }
    return nullptr;
}

static void NativeChildProcessExit(int32_t pid, int32_t signal)
{}

static napi_value RegisterNativeChildProcessExitCallbackNoError(napi_env env, napi_callback_info info)
{
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_RegisterNativeChildProcessExitCallback(NativeChildProcessExit);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value RegisterNativeChildProcessExitCallbackInvalidParam(napi_env env, napi_callback_info info)
{
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_RegisterNativeChildProcessExitCallback(nullptr);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value UnregisterNativeChildProcessExitCallbackNoError(napi_env env, napi_callback_info info)
{
    OH_Ability_RegisterNativeChildProcessExitCallback(NativeChildProcessExit);
    Ability_NativeChildProcess_ErrCode ret =
        OH_Ability_UnregisterNativeChildProcessExitCallback(NativeChildProcessExit);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value UnregisterNativeChildProcessExitCallbackInvalidParam(napi_env env, napi_callback_info info)
{
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_UnregisterNativeChildProcessExitCallback(nullptr);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value UnregisterNativeChildProcessExitCallbackNotExist(napi_env env, napi_callback_info info)
{
    void* errPtr = reinterpret_cast<void*>(0x12345678);
    auto data = reinterpret_cast<OH_Ability_OnNativeChildProcessExit>(errPtr);
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_UnregisterNativeChildProcessExitCallback(data);
    if(ret==NCP_ERR_CALLBACK_NOT_EXIST)
        {
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
        }
    return nullptr;
}

static napi_value DestroyWantNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    AbilityBase_ErrorCode ret = OH_AbilityBase_DestroyWant(want);
    if(ret == ABILITY_BASE_ERROR_CODE_NO_ERROR)
        {
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
        }
    return nullptr;
}

static napi_value SetWantElementNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    AbilityBase_ErrorCode ret = OH_AbilityBase_SetWantElement(want, element);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetWantElementNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    AbilityBase_ErrorCode ret = OH_AbilityBase_GetWantElement(want, &element);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetWantCharParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    AbilityBase_ErrorCode ret = OH_AbilityBase_SetWantCharParam(want, "key", "value");
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetWantCharParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    OH_AbilityBase_SetWantCharParam(want, "key", "set second value");
    char outvalue[30];
    AbilityBase_ErrorCode ret = OH_AbilityBase_GetWantCharParam(want, "key", outvalue, sizeof(outvalue));
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value AddWantFdNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    AbilityBase_ErrorCode ret = OH_AbilityBase_AddWantFd(want, "fd", 100);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetWantUriNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    AbilityBase_ErrorCode ret = OH_AbilityBase_SetWantUri(want, "file://xxxxxxxx");
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetWantUriNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    char uriBuffer[32] = {0};
    AbilityBase_ErrorCode ret = OH_AbilityBase_GetWantUri(want, uriBuffer, sizeof(uriBuffer));
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetWantInt32ParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    AbilityBase_ErrorCode ret = OH_AbilityBase_SetWantInt32Param(want, "keyInt", 32);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetWantInt32ParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    const int KEY_INT_VALUE = 32;
    OH_AbilityBase_SetWantInt32Param(want, "keyInt", KEY_INT_VALUE);
    int32_t outElement = 0;
    AbilityBase_ErrorCode ret = OH_AbilityBase_GetWantInt32Param(want, "keyInt", &outElement);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetWantBoolParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    bool flag = false;
    AbilityBase_ErrorCode ret = OH_AbilityBase_SetWantBoolParam(want, "keyBool", flag);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetWantBoolParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    bool flag = false;
    OH_AbilityBase_SetWantBoolParam(want, "keyBool", flag);
    bool outElement = 0;
    AbilityBase_ErrorCode ret = OH_AbilityBase_GetWantBoolParam(want, "keyBool", &outElement);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetWantDoubleParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    double param = 1.0;
    AbilityBase_ErrorCode ret = OH_AbilityBase_SetWantDoubleParam(want, "keyDouble", param);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetWantDoubleParamNoError(napi_env env, napi_callback_info info)
{
    AbilityBase_Element element;
    element.abilityName = const_cast<char *>("EntryAbility");
    element.bundleName = const_cast<char *>("com.acts.ActsCapabilityStartSelfUIAbilityTest");
    element.moduleName = const_cast<char *>("entry");
    AbilityBase_Want* want = OH_AbilityBase_CreateWant(element);
    double outElement = 0;
    OH_AbilityBase_SetWantDoubleParam(want, "keyDouble", outElement);
    AbilityBase_ErrorCode ret = OH_AbilityBase_GetWantDoubleParam(want, "keyDouble", &outElement);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value AbilityRuntimeAreaModeEl2Code(napi_env env, napi_callback_info info)
{
    AbilityRuntime_AreaMode para = ABILITY_RUNTIME_AREA_MODE_EL2;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_ApplicationContextGetAreaMode(&para);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value AbilityRuntimeAreaModeEl3Code(napi_env env, napi_callback_info info)
{
    AbilityRuntime_AreaMode para = ABILITY_RUNTIME_AREA_MODE_EL3;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_ApplicationContextGetAreaMode(&para);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value AbilityRuntimeAreaModeEl4Code(napi_env env, napi_callback_info info)
{
    AbilityRuntime_AreaMode para = ABILITY_RUNTIME_AREA_MODE_EL4;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_ApplicationContextGetAreaMode(&para);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value AbilityRuntimeAreaModeEl5Code(napi_env env, napi_callback_info info)
{
    AbilityRuntime_AreaMode para = ABILITY_RUNTIME_AREA_MODE_EL5;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_ApplicationContextGetAreaMode(&para);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value AbilityRuntimeHideUponStartCode(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartVisibility visibility = AbilityRuntime_StartVisibility::ABILITY_RUNTIME_HIDE_UPON_START;
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode errCode = OH_AbilityRuntime_SetStartOptionsStartVisibility(options, visibility);
    napi_value napiRet;
    napi_create_int32(env, errCode, &napiRet);
    return napiRet;
}

static napi_value DestroyStartOptionsNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_DestroyStartOptions(&options);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsWindowModeNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_SetStartOptionsWindowMode(options, ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsWindowModeNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_WindowMode windowMode = ABILITY_RUNTIME_WINDOW_MODE_UNDEFINED;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsWindowMode(options, windowMode);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsDisplayIdNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_SetStartOptionsDisplayId(options, ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsDisplayIdParamInvalid(napi_env env, napi_callback_info info)
{
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_SetStartOptionsDisplayId(nullptr, ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsDisplayIdNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t displayId = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsDisplayId(options, displayId);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsWithAnimationNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_SetStartOptionsWithAnimation(options, ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsWithAnimationNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    bool withAnimation = false;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsWithAnimation(options, withAnimation);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsWindowLeftNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_SetStartOptionsWindowLeft(options, ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsWindowLeftNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsWindowLeft(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsWindowTopNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_SetStartOptionsWindowTop(options, ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsWindowTopNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsWindowTop(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsWindowHeightNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsWindowHeight(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsWindowHeightNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsWindowHeight(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsWindowWidthNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsWindowWidth(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsWindowWidthNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsWindowWidth(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsStartVisibilityNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_StartVisibility visibility = AbilityRuntime_StartVisibility::ABILITY_RUNTIME_SHOW_UPON_START;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsStartVisibility(options, visibility);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsStartVisibilityNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_StartVisibility visibility = AbilityRuntime_StartVisibility::ABILITY_RUNTIME_SHOW_UPON_START;
    OH_AbilityRuntime_SetStartOptionsStartVisibility(options, visibility);
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsStartVisibility(options,visibility);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsStartWindowIconNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    uint8_t data[96];
    size_t dataSize = 96;
    for (int i = 0; i < dataSize; i++) {
        data[i] = i + 1;
    }
    
    OH_Pixelmap_InitializationOptions *createOpts = nullptr;
    int32_t setWidth = 6;
    int32_t setHeight = 4;
    OH_PixelmapInitializationOptions_Create(&createOpts);
    OH_PixelmapInitializationOptions_SetWidth(createOpts, setWidth);
    OH_PixelmapInitializationOptions_SetHeight(createOpts, setHeight);
    OH_PixelmapInitializationOptions_SetPixelFormat(createOpts, PIXEL_FORMAT_RGBA_8888);
    OH_PixelmapInitializationOptions_SetAlphaType(createOpts, PIXELMAP_ALPHA_TYPE_UNKNOWN);

    OH_PixelmapNative *startWindowIcon = nullptr;
    Image_ErrorCode imgErrCode = OH_PixelmapNative_CreatePixelmap(data, dataSize, createOpts, &startWindowIcon);
    if (imgErrCode != IMAGE_SUCCESS) {
        OH_PixelmapInitializationOptions_Release(createOpts);
    }
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsStartWindowIcon(options, startWindowIcon);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsStartWindowIconNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    uint8_t data[96];
    size_t dataSize = 96;
    for (int i = 0; i < dataSize; i++) {
        data[i] = i + 1;
    }

    OH_Pixelmap_InitializationOptions *createOpts = nullptr;
    int32_t setWidth = 6;
    int32_t setHeight = 4;
    OH_PixelmapInitializationOptions_Create(&createOpts);
    OH_PixelmapInitializationOptions_SetWidth(createOpts, setWidth);
    OH_PixelmapInitializationOptions_SetHeight(createOpts, setHeight);
    OH_PixelmapInitializationOptions_SetPixelFormat(createOpts, PIXEL_FORMAT_RGBA_8888);
    OH_PixelmapInitializationOptions_SetAlphaType(createOpts, PIXELMAP_ALPHA_TYPE_UNKNOWN);

    OH_PixelmapNative *startWindowIcon = nullptr;
    Image_ErrorCode imgErrCode = OH_PixelmapNative_CreatePixelmap(data, dataSize, createOpts, &startWindowIcon);
    if (imgErrCode != IMAGE_SUCCESS) {
        OH_PixelmapInitializationOptions_Release(createOpts);
    }
    OH_AbilityRuntime_SetStartOptionsStartWindowIcon(options, startWindowIcon);
    startWindowIcon = nullptr;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsStartWindowIcon(options, &startWindowIcon);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsStartWindowBackgroundColorNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor(options, "#00000000");
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsStartWindowBackgroundColorNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    char originalStartWindowBackgroundColor[] = "";
    OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor(startOptions, originalStartWindowBackgroundColor);
    char *startWindowBackgroundColor = nullptr;
    size_t size = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColor(
        startOptions, &startWindowBackgroundColor, size);
    
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsSupportedWindowModesNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_SupportedWindowMode supportedWindowModes[3] = {
        ABILITY_RUNTIME_SUPPORTED_WINDOW_MODE_FULL_SCREEN,
        ABILITY_RUNTIME_SUPPORTED_WINDOW_MODE_SPLIT,
        ABILITY_RUNTIME_SUPPORTED_WINDOW_MODE_FLOATING,
    };
    size_t supportedWindowModesSize = 3;
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_SetStartOptionsSupportedWindowModes(options, supportedWindowModes, supportedWindowModesSize);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsSupportedWindowModesNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_SupportedWindowMode *supportedWindowModes = nullptr;
    size_t size = 0;
    AbilityRuntime_ErrorCode ret =
        OH_AbilityRuntime_GetStartOptionsSupportedWindowModes(options, &supportedWindowModes, size);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsMinWindowWidthNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsMinWindowWidth(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsMinWindowWidthNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsMinWindowWidth(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsMaxWindowWidthNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsMaxWindowWidth(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsMaxWindowWidthNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsMaxWindowWidth(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsMinWindowHeightNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsMinWindowHeight(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsMinWindowHeightNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsMinWindowHeight(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsMinWindowHeightParamInvalid(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsMinWindowHeight(nullptr, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value SetStartOptionsMaxWindowHeightNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_SetStartOptionsMaxWindowHeight(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

static napi_value GetStartOptionsMaxWindowHeightNoError(napi_env env, napi_callback_info info)
{
    AbilityRuntime_StartOptions *options = OH_AbilityRuntime_CreateStartOptions();
    int32_t num = 0;
    AbilityRuntime_ErrorCode ret = OH_AbilityRuntime_GetStartOptionsMaxWindowHeight(options, num);
    napi_value napiRet;
    napi_create_int32(env, ret, &napiRet);
    return napiRet;
}

napi_property_descriptor desc[] = {
    {"DestroyChildProcessConfigsParamInvalid",
        nullptr, DestroyChildProcessConfigsParamInvalid, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DestroyChildProcessConfigsNoError",
        nullptr, DestroyChildProcessConfigsNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetIsolationModeConfigsInvalidParam",
        nullptr, SetIsolationModeConfigsInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetIsolationModeConfigsNoError",
        nullptr, SetIsolationModeConfigsNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetProcessNameConfigsInvalidParam",
        nullptr, SetProcessNameConfigsInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetProcessNameConfigsNoError",
        nullptr, SetProcessNameConfigsNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"RegisterNativeChildProcessExitCallbackNoError",
        nullptr, RegisterNativeChildProcessExitCallbackNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"RegisterNativeChildProcessExitCallbackInvalidParam", nullptr,
        RegisterNativeChildProcessExitCallbackInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"UnregisterNativeChildProcessExitCallbackNoError",
        nullptr, UnregisterNativeChildProcessExitCallbackNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"UnregisterNativeChildProcessExitCallbackInvalidParam", nullptr,
        UnregisterNativeChildProcessExitCallbackInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"UnregisterNativeChildProcessExitCallbackNotExist", nullptr,
        UnregisterNativeChildProcessExitCallbackNotExist, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DestroyWantNoError",
        nullptr, DestroyWantNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetWantElementNoError",
        nullptr, SetWantElementNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetWantElementNoError",
        nullptr, GetWantElementNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetWantCharParamNoError",
        nullptr, SetWantCharParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetWantCharParamNoError",
        nullptr, GetWantCharParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"AddWantFdNoError",
        nullptr, AddWantFdNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetWantUriNoError",
        nullptr, SetWantUriNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetWantUriNoError",
        nullptr, GetWantUriNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetWantInt32ParamNoError",
        nullptr, SetWantInt32ParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetWantInt32ParamNoError",
        nullptr, GetWantInt32ParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetWantBoolParamNoError",
        nullptr, SetWantBoolParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetWantBoolParamNoError",
        nullptr, GetWantBoolParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetWantDoubleParamNoError",
        nullptr, SetWantDoubleParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetWantDoubleParamNoError",
        nullptr, GetWantDoubleParamNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"AbilityRuntimeAreaModeEl2Code",
        nullptr, AbilityRuntimeAreaModeEl2Code, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"AbilityRuntimeAreaModeEl3Code",
        nullptr, AbilityRuntimeAreaModeEl3Code, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"AbilityRuntimeAreaModeEl4Code",
        nullptr, AbilityRuntimeAreaModeEl4Code, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"AbilityRuntimeAreaModeEl5Code",
        nullptr, AbilityRuntimeAreaModeEl5Code, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"AbilityRuntimeHideUponStartCode",
        nullptr, AbilityRuntimeHideUponStartCode, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DestroyStartOptionsNoError",
        nullptr, DestroyStartOptionsNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsWindowModeNoError",
        nullptr, SetStartOptionsWindowModeNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsWindowModeNoError",
        nullptr, GetStartOptionsWindowModeNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsDisplayIdNoError",
        nullptr, SetStartOptionsDisplayIdNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsDisplayIdParamInvalid",
        nullptr, SetStartOptionsDisplayIdParamInvalid, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsDisplayIdNoError",
        nullptr, GetStartOptionsDisplayIdNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsWithAnimationNoError",
        nullptr, SetStartOptionsWithAnimationNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsWithAnimationNoError",
        nullptr, GetStartOptionsWithAnimationNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsWindowLeftNoError",
        nullptr, SetStartOptionsWindowLeftNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsWindowLeftNoError",
        nullptr, GetStartOptionsWindowLeftNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsWindowTopNoError",
        nullptr, SetStartOptionsWindowTopNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsWindowTopNoError",
        nullptr, GetStartOptionsWindowTopNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsWindowHeightNoError",
        nullptr, SetStartOptionsWindowHeightNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsWindowHeightNoError",
        nullptr, GetStartOptionsWindowHeightNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsWindowWidthNoError",
        nullptr, SetStartOptionsWindowWidthNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsWindowWidthNoError",
        nullptr, GetStartOptionsWindowWidthNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsStartVisibilityNoError",
        nullptr, SetStartOptionsStartVisibilityNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsStartVisibilityNoError",
        nullptr, GetStartOptionsStartVisibilityNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsStartWindowIconNoError",
        nullptr, SetStartOptionsStartWindowIconNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsStartWindowIconNoError",
        nullptr, GetStartOptionsStartWindowIconNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsStartWindowBackgroundColorNoError", nullptr,
        SetStartOptionsStartWindowBackgroundColorNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsStartWindowBackgroundColorNoError", nullptr,
        GetStartOptionsStartWindowBackgroundColorNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsSupportedWindowModesNoError",
        nullptr, SetStartOptionsSupportedWindowModesNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsSupportedWindowModesNoError",
        nullptr, GetStartOptionsSupportedWindowModesNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsMinWindowWidthNoError",
        nullptr, SetStartOptionsMinWindowWidthNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsMinWindowWidthNoError",
        nullptr, GetStartOptionsMinWindowWidthNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsMaxWindowWidthNoError",
        nullptr, SetStartOptionsMaxWindowWidthNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsMaxWindowWidthNoError",
        nullptr, GetStartOptionsMaxWindowWidthNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsMinWindowHeightNoError",
        nullptr, SetStartOptionsMinWindowHeightNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsMinWindowHeightNoError",
        nullptr, GetStartOptionsMinWindowHeightNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsMinWindowHeightParamInvalid",
        nullptr, GetStartOptionsMinWindowHeightParamInvalid, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SetStartOptionsMaxWindowHeightNoError",
        nullptr, SetStartOptionsMaxWindowHeightNoError, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetStartOptionsMaxWindowHeightNoError",
        nullptr, GetStartOptionsMaxWindowHeightNoError, nullptr, nullptr, nullptr, napi_default, nullptr}
    };

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "osaccountndk",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&demoModule); }
