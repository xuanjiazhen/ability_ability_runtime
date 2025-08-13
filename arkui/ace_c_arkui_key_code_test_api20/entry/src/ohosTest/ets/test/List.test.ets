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
import ArkUIKeyCodeNdkTest1 from './arkuikeycodetest/ArkUIKeyCodeTest1.test'
import ArkUIKeyCodeNdkTest2 from './arkuikeycodetest/ArkUIKeyCodeTest2.test'
import ArkUIKeyCodeNdkTest3 from './arkuikeycodetest/ArkUIKeyCodeTest3.test'
import ArkUIKeyCodeNdkTest4 from './arkuikeycodetest/ArkUIKeyCodeTest4.test'
import FocusControlTest from './foucscontroltest/FocusRequestTest.test'
import KeyEventTest from './keyeventtest/KeyEventTest.test'
import NodeEventTest from './nodeeventtest/NodeEventTest.test'
import NodeAdapterTest from './nodeadaptertest/NodeAdapterTest.test'
import NodeContentTest from './nodecontenttest/NodecontentTest.test'
import SnapShotTest from './snapshottest/SnapShotTest.test'
import RunTaskInscopeTest from './scopetest/RunTaskInScopeTest.test'
import NodeNapiTest from './nodenapitest/NodeNapiTest.test'
import NodeNapiTest1 from './nodenapitest/NodeNapiTest1.test'
import NodeNapiTest2 from './nodenapitest/NodeNapiTest2.test'
import NodeNapiTest3 from './nodenapitest/NodeNapiTest3.test'
import NodeNapiTest4 from './nodenapitest/NodeNapiTest4.test'
import NavigationTest from './nodenapitest/NavigationTest.test'
import NavDestinationTest from './nodenapitest/NavDestinationTest.test'
import GetRouterTest from './nodenapitest/GetRouterTest.test'
import NodeUtilsTest from './nodeutilstest/NodeUtilsTest.test'
import NodeUtilsTest1 from './nodeutilstest/NodeUtilsTest1.test'
import NodeUtilsTest2 from './nodeutilstest/NodeUtilsTest2.test'

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
  ArkUIKeyCodeNdkTest1()
  ArkUIKeyCodeNdkTest2()
  ArkUIKeyCodeNdkTest3()
  ArkUIKeyCodeNdkTest4()
  FocusControlTest()
  KeyEventTest()
  NodeEventTest()
  NodeAdapterTest()
  NodeContentTest()
  SnapShotTest()
  RunTaskInscopeTest()
  NodeNapiTest()
  NodeNapiTest1()
  NodeNapiTest2()
  NodeNapiTest3()
  NodeNapiTest4()
  NavigationTest()
  NavDestinationTest()
  GetRouterTest()
  NodeUtilsTest1()
  NodeUtilsTest()
  NodeUtilsTest2()
}
