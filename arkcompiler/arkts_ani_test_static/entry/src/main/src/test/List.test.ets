/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import versionInfoTest from "./ActsVersionInfo.test";
import classOperationsTest from "./ActsClassOperations.test";
import objectOperationsTest from "./ActsObjectOperations.test"
import GlobalAndLocalReferencesTest from "./ActsGlobalAndLocalReferences.test.ets"
import globalAndLocalReferencesTest from "./ActsGlobalAndLocalReferences.test.ets"
import accessingStaticFieldsTest from "./ActsAccessingStaticFields.test.ets"
import callingStaticMethodsTest from "./ActsClassCallingStaticMethods.test";
import stringOperationsTest from "./ActsStringOperations.test"
import ArrayOperationsTest from "./ActsArrayOperations.test"
import ScopeSupportTest from "./ActsScopeSupport.test"
import JSFeatureTest from "./ActsJSFeature.test"
import EnvOperationsTest from "./ActsEnvOperations.test"
import accessingFiledsOfObjectsTest from "./ActsAccessingFiledsOfObjects.test"
import callingInstanceMethodsTest from "./ActsCallingInstanceMethods.test"
import variableOperationsTest from "./ActsVariableOperations.test";
import moduleSupportTest from "./ActsModuleSupport.test";
import nameSpaceSupportTest from "./ActsNameSpaceSupport.test";
import enumOperationsTest from "./ActsEnumOperations.test";
import tupleOperationsTest from "./ActsTupleOperations.test";
import vmOpsTest from "./ActsVmOps.test"
import exceptionTest from "./ActsException.test"
import promiseTest from "./ActsPromise.test"
import weakGlobalReferencesTest from "./ActsWeakGlobalReferences.test"

export default function testsuite() {
    versionInfoTest();
    classOperationsTest();
    objectOperationsTest();
    globalAndLocalReferencesTest();
    accessingStaticFieldsTest();
    callingStaticMethodsTest();
    stringOperationsTest();
    ArrayOperationsTest();
    ScopeSupportTest();
    JSFeatureTest();
    EnvOperationsTest();
    accessingFiledsOfObjectsTest();
    callingInstanceMethodsTest();
    variableOperationsTest();
    moduleSupportTest();
    nameSpaceSupportTest();
    enumOperationsTest();
    tupleOperationsTest();
    vmOpsTest();
    promiseTest();
    exceptionTest();
    weakGlobalReferencesTest();
}
