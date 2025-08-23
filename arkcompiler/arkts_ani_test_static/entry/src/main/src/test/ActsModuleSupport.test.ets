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

import { describe, it, expect, TestType, Size, Level } from "../../../hypium/index";
import hilog from '@ohos.hilog'
import Utils from './Util.test';
import { Foo } from '../ets/Index'

let domain: number = 0x0000;
let tag: string = 'testTag';

export default function moduleSupportTest() {

  describe("ActsModuleSupportTest", (): void => {
    try {
      loadLibrary("entry")
    } catch(e: Error) {
      console.log('loadFailed', e.message);
    }

    /*
     * @tc.number    : Module_FindNamespace_0100
     * @tc.name      : Module_FindNamespace_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Module_FindNamespace_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Module_FindNamespace_0100 start');
        let cls = new Foo();
        let result = cls.test_Module_FindNamespace();
        expect(result).assertTrue();
        console.log('Module_FindNamespace_0100 done');
        done();
        console.log('Module_FindNamespace_0100 end');
      })

    /*
     * @tc.number    : Module_FindClass_0100
     * @tc.name      : Module_FindClass_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Module_FindClass_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Module_FindClass_0100 start');
        let cls = new Foo();
        let result = cls.test_Module_FindClass();
        expect(result).assertTrue();
        console.log('Module_FindClass_0100 done');
        done();
        console.log('Module_FindClass_0100 end');
      })

    /*
     * @tc.number    : Module_FindEnum_0100
     * @tc.name      : Module_FindEnum_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Module_FindEnum_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Module_FindEnum_0100 start');
        let cls = new Foo();
        let result = cls.test_Module_FindEnum();
        expect(result).assertTrue();
        console.log('Module_FindEnum_0100 done');
        done();
        console.log('Module_FindEnum_0100 end');
      })

    /*
     * @tc.number    : Module_FindFunction_0100
     * @tc.name      : Module_FindFunction_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Module_FindFunction_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Module_FindFunction_0100 start');
        let cls = new Foo();
        let result = cls.test_Module_FindFunction();
        expect(result).assertTrue();
        console.log('Module_FindFunction_0100 done');
        done();
        console.log('Module_FindFunction_0100 end');
      })

    /*
     * @tc.number    : Module_FindVariable_0100
     * @tc.name      : Module_FindVariable_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Module_FindVariable_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Module_FindVariable_0100 start');
        let cls = new Foo();
        let result = cls.test_Module_FindVariable();
        expect(result).assertTrue();
        console.log('Module_FindVariable_0100 done');
        done();
        console.log('Module_FindVariable_0100 end');
      })
  })
}