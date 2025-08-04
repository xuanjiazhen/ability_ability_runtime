/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "common/common.h"
#include "darkConfig/SetForceDarkConfigTest.cpp"
#include "arkTSEnv/InitModuleForArkTSEnvTest.cpp"
#include "renderNode/RenderNodeAttributeTest.cpp"

namespace ArkUICapiTest
{
    EXTERN_C_START
    static napi_value Init(napi_env env, napi_value exports)
    {
        if ((env == nullptr) || (exports == nullptr))
        {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init", "env or exports is null");
            return nullptr;
        }

        napi_property_descriptor desc[] = {
            {"TestSetForceDarkConfig", nullptr, TestSetForceDarkConfig, nullptr, nullptr, nullptr,
                napi_default, nullptr},
            {"TestInitModuleForArkTSEnv", nullptr, TestInitModuleForArkTSEnv, nullptr, nullptr, nullptr,
                napi_default, nullptr},
            {"TestBackgroundColor", nullptr, TestBackgroundColor, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestBorderColor", nullptr, TestBorderColor, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestBorderRadius", nullptr, TestBorderRadius, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestBorderStyle", nullptr, TestBorderStyle, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestBorderWidth", nullptr, TestBorderWidth, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestBounds", nullptr, TestBounds, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestClipToBounds", nullptr, TestClipToBounds, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestClipToFrame", nullptr, TestClipToFrame, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestOpacity", nullptr, TestOpacity, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestPivot", nullptr, TestPivot, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestPosition", nullptr, TestPosition, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestRotation", nullptr, TestRotation, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestScale", nullptr, TestScale, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestSetDrawRegion", nullptr, TestSetDrawRegion, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestSetMarkNodeGroup", nullptr, TestSetMarkNodeGroup, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestShadowAlpha", nullptr, TestShadowAlpha, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestShadowColor", nullptr, TestShadowColor, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestShadowElevation", nullptr, TestShadowElevation, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestShadowOffset", nullptr, TestShadowOffset, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestShadowRadius", nullptr, TestShadowRadius, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestSize", nullptr, TestSize, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestTransform", nullptr, TestTransform, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestTranslation", nullptr, TestTranslation, nullptr, nullptr, nullptr, napi_default, nullptr},
            
            {"TestAddRenderNode001", nullptr, TestAddRenderNode001, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestAddRenderNode002", nullptr, TestAddRenderNode002, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestAddRenderNode003", nullptr, TestAddRenderNode003, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestRemoveRenderNode001", nullptr, TestRemoveRenderNode001, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestRemoveRenderNode002", nullptr, TestRemoveRenderNode002, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"TestClearRenderNodeChildren001", nullptr, TestClearRenderNodeChildren001, nullptr, nullptr, nullptr,
                napi_default, nullptr},
            {"TestClearRenderNodeChildren002", nullptr, TestClearRenderNodeChildren002, nullptr, nullptr, nullptr,
                napi_default, nullptr},
            {"TestInvalidate", nullptr, TestInvalidate, nullptr, nullptr, nullptr, napi_default, nullptr},
        };
        if (napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc) != napi_ok)
        {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init", "napi_define_properties failed");
            return nullptr;
        }
        return exports;
    }
    EXTERN_C_END

    static napi_module nativefuncModule = {.nm_version = 1,
                                           .nm_flags = 0,
                                           .nm_filename = nullptr,
                                           .nm_register_func = Init,
                                           .nm_modname = "nativefunc",
                                           .nm_priv = ((void *)0),
                                           .reserved = {0}};

    extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&nativefuncModule); }
} // namespace ArkUICapiTest
