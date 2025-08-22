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

#ifndef TEST2_H
#define TEST2_H

#include <arkui/native_node_napi.h>
#include <napi/native_api.h>
#include <js_native_api_types.h>

namespace AnimatorOptionTest {

// Delay related functions
napi_value KeyframeSetDelay0100(napi_env env, napi_callback_info info);
napi_value KeyframeSetDelay0200(napi_env env, napi_callback_info info);

// Iterations related functions
napi_value KeyframeSetIterations0100(napi_env env, napi_callback_info info);
napi_value KeyframeSetIterations0200(napi_env env, napi_callback_info info);
napi_value KeyframeSetIterations0300(napi_env env, napi_callback_info info);

// Finish callback related functions
napi_value KeyframeRegisterOnFinishCallback0100(napi_env env, napi_callback_info info);
napi_value KeyframeRegisterOnFinishCallback0200(napi_env env, napi_callback_info info);

// Frame rate related functions
napi_value KeyframeSetExpectedFrameRate0100(napi_env env, napi_callback_info info);
napi_value KeyframeSetExpectedFrameRate0200(napi_env env, napi_callback_info info);
napi_value KeyframeSetExpectedFrameRate0300(napi_env env, napi_callback_info info);

// Duration related functions
napi_value KeyframeSetDuration0100(napi_env env, napi_callback_info info);
napi_value KeyframeSetDuration0200(napi_env env, napi_callback_info info);
napi_value KeyframeSetDuration0300(napi_env env, napi_callback_info info);
napi_value KeyframeSetDuration0400(napi_env env, napi_callback_info info);
napi_value KeyframeSetDuration0500(napi_env env, napi_callback_info info);

// Curve related functions
napi_value KeyframeSetCurve0100(napi_env env, napi_callback_info info);
napi_value KeyframeSetCurve0200(napi_env env, napi_callback_info info);
napi_value KeyframeSetCurve0300(napi_env env, napi_callback_info info);
napi_value KeyframeSetCurve0400(napi_env env, napi_callback_info info);

// Event callback related functions
napi_value KeyframeRegisterOnEventCallback0100(napi_env env, napi_callback_info info);
napi_value KeyframeRegisterOnEventCallback0200(napi_env env, napi_callback_info info);
napi_value KeyframeRegisterOnEventCallback0300(napi_env env, napi_callback_info info);
napi_value KeyframeRegisterOnEventCallback0400(napi_env env, napi_callback_info info);

// Duration related functions
napi_value AnimatorOptionSetDuration0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetDuration0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetDuration0300(napi_env env, napi_callback_info info);

// Delay related functions
napi_value AnimatorOptionSetDelay0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetDelay0200(napi_env env, napi_callback_info info);

// Iterations related functions
napi_value AnimatorOptionSetIterations0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetIterations0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetIterations0300(napi_env env, napi_callback_info info);

// Fill mode related functions
napi_value AnimatorOptionSetFill0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetFill0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetFill0300(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetFill0400(napi_env env, napi_callback_info info);

// Direction related functions
napi_value AnimatorOptionSetDirection0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetDirection0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetDirection0300(napi_env env, napi_callback_info info);

// Curve related functions
napi_value AnimatorOptionSetCurve0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetCurve0200(napi_env env, napi_callback_info info);

// Begin position related functions
napi_value AnimatorOptionSetBegin0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetBegin0200(napi_env env, napi_callback_info info);

// End position related functions
napi_value AnimatorOptionSetEnd0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetEnd0200(napi_env env, napi_callback_info info);

// Frame rate range related functions
napi_value AnimatorOptionSetExpectedFrameRateRange0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetExpectedFrameRateRange0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetExpectedFrameRateRange0300(napi_env env, napi_callback_info info);

// Keyframe related functions
napi_value AnimatorOptionSetKeyframe0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetKeyframe0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetKeyframe0300(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetKeyframe0400(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetKeyframe0500(napi_env env, napi_callback_info info);

// Keyframe curve related functions
napi_value AnimatorOptionSetKeyframeCurve0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetKeyframeCurve0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetKeyframeCurve0300(napi_env env, napi_callback_info info);
napi_value AnimatorOptionSetKeyframeCurve0400(napi_env env, napi_callback_info info);

//RegisterOnFrameCallback  related functions
napi_value AnimatorOptionRegisterOnFrameCallback0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnFrameCallback0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnFrameCallback0300(napi_env env, napi_callback_info info);

//RegisterOnFinishCallback  related functions
napi_value AnimatorOptionRegisterOnFinishCallback0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnFinishCallback0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnFinishCallback0300(napi_env env, napi_callback_info info);

//RegisterOnCancelCallback  related functions
napi_value AnimatorOptionRegisterOnCancelCallback0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnCancelCallback0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnCancelCallback0300(napi_env env, napi_callback_info info);

//RegisterOnRepeatCallback  related functions
napi_value AnimatorOptionRegisterOnRepeatCallback0100(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnRepeatCallback0200(napi_env env, napi_callback_info info);
napi_value AnimatorOptionRegisterOnRepeatCallback0300(napi_env env, napi_callback_info info);

} // namespace AnimatorOptionTest

#endif // TEST2_H