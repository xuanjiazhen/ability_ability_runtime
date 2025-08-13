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
 
import KeyCodeNdkTest from './nativekeycode/KeyCodeTest.test'
import KeyCodeNdkTest1 from './nativekeycode/KeyCodeTest1.test'
import KeyCodeNdkTest2 from './nativekeycode/KeyCodeTest2.test'
import KeyCodeNdkTest3 from './nativekeycode/KeyCodeTest3.test'
import KeyCodeNdkTest4 from './nativekeycode/KeyCodeTest4.test'
import NativeXComponent from './nativexcomponent/NativeXComponent.test'
import NodeAttribute from './nativexcomponent/NodeAttribute.test'
import TouchPointTest from './nativexcomponent/TouchPoint.test'
import XComponentMouseTest from './nativexcomponent/XComponentMouse.test'
import AccessibilityNdkTest1 from './accessibilitytest/AccessibilityTest1.test'
import AccessibilityNdkTest2 from './accessibilitytest/AccessibilityTest2.test'
import AccessibilityNdkTest3 from './accessibilitytest/AccessibilityTest3.test'
import AccessibilityNdkTest4 from './accessibilitytest/AccessibilityTest4.test'
import AccessibilityNdkTest5 from './accessibilitytest/AccessibilityTest5.test'
import AccessibilityNdkTest6 from './accessibilitytest/AccessibilityTest6.test'
import KeyframeAnimateTest from './animatetest/KeyframeAnimateTest.test'
import AnimatorOptionTest1 from './animatetest/AnimatorOptionTest1.test'
import AnimatorOptionTest2 from './animatetest/AnimatorOptionTest2.test'

export default function testsuite() {
  KeyCodeNdkTest()
  KeyCodeNdkTest1()
  KeyCodeNdkTest2()
  KeyCodeNdkTest3()
  KeyCodeNdkTest4()
  NativeXComponent()
  TouchPointTest()
  XComponentMouseTest()
  NodeAttribute()
  AccessibilityNdkTest1()
  AccessibilityNdkTest2()
  AccessibilityNdkTest3()
  AccessibilityNdkTest4()
  AccessibilityNdkTest5()
  AccessibilityNdkTest6()
  KeyframeAnimateTest()
  AnimatorOptionTest1()
  AnimatorOptionTest2()
}
