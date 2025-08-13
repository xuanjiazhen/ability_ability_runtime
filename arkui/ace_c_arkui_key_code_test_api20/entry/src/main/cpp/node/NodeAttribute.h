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

#ifndef NDKXCOMPONENT_ATTRIBUTE_H
#define NDKXCOMPONENT_ATTRIBUTE_H

#include "component/XComponent.h"

#define XC ArkUICapiTest::XComponent

namespace NodeAttribute {

static XC ColumnComponent = XC(ARKUI_NODE_COLUMN);
void InitNode();

napi_value CreateAttributeNativeNode(napi_env env, napi_callback_info info);
napi_value CheckNodeTextHalfLeading(napi_env env, napi_callback_info info);
napi_value CheckNodeTextLineCount(napi_env env, napi_callback_info info);
napi_value CheckNodeTextOptimizeTrailingSpace(napi_env env, napi_callback_info info);
napi_value CheckNodeTextLinearGradient(napi_env env, napi_callback_info info);
napi_value CheckNodeTextRadialGradient(napi_env env, napi_callback_info info);
napi_value CheckNodeTextVerticalAlign(napi_env env, napi_callback_info info);
napi_value CheckNodeTextInputHalfLeading(napi_env env, napi_callback_info info);
napi_value CheckNodeTextInputKeyboardAppearance(napi_env env, napi_callback_info info);
napi_value CheckNodeTextInputEnableFillAnimation(napi_env env, napi_callback_info info);
napi_value CheckNodeTextInputLineHeight(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaInputFilter(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaContentType(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaHalfLeading(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaKeyboardAppearance(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaMaxLines(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaLineSpacing(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaMinLines(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaMaxLinesWithScroll(napi_env env, napi_callback_info info);
napi_value CheckNodeTextAreaLineHeight(napi_env env, napi_callback_info info);
napi_value CheckNodeTextPickSelectedBackgroundStyle(napi_env env, napi_callback_info info);
};

#endif //NDKXCOMPONENT_ATTRIBUTE_H
