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

export default function classOperationsTest() {

  describe("ActsClassOperationsTest", (): void => {
    try {
      loadLibrary("entry")
    } catch(e: Error) {
      console.log('loadFailed', e.message);
    }

    /*
     * @tc.number    : findClass_static_0100
     * @tc.name      : findClass_static_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("findClass_static_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
      console.log('findClass_static_0100 start');
      let cls = new Foo();
      let result = cls.test_FindClass();
      expect(result).assertTrue();
      console.log('findClass_static_0100 done');
      done();
      console.log('findClass_static_0100 end');
    })

    /*
     * @tc.number    : class_FindStaticMethodBoolean_0100
     * @tc.name      : class_FindStaticMethodBoolean_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("class_FindStaticMethodBoolean_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
      console.log('class_FindStaticMethodBoolean_0100 start');
      let cls = new Foo();
      let result = cls.test_Class_FindStaticMethodBoolean();
      expect(result).assertTrue();
      console.log('class_FindStaticMethodBoolean_0100 done');
      done();
      console.log('class_FindStaticMethodBoolean_0100 end');
    })

    /*
     * @tc.number    : class_FindStaticMethodInt_0100
     * @tc.name      : class_FindStaticMethodInt_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("class_FindStaticMethodInt_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
      console.log('class_FindStaticMethodInt_0100 start');
      let cls = new Foo();
      let result = cls.test_Class_FindStaticMethodInt();
      expect(result).assertTrue();
      console.log('class_FindStaticMethodInt_0100 done');
      done();
      console.log('class_FindStaticMethodInt_0100 end');
    })

    /*
     * @tc.number    : Class_BindNativeMethods_0100
     * @tc.name      : Class_BindNativeMethods_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Class_BindNativeMethods_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Class_BindNativeMethods_0100 start');
        let cls = new Foo();
        let result = cls.test_Class_BindNativeMethods();
        expect(result).assertTrue();
        console.log('Class_BindNativeMethods_0100 done');
        done();
        console.log('Class_BindNativeMethods_0100 end');
      })

    /*
     * @tc.number    : Type_GetSuperClass_0100
     * @tc.name      : Type_GetSuperClass_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Type_GetSuperClass_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Type_GetSuperClass_0100 start');
        let cls = new Foo();
        let result = cls.test_Type_GetSuperClass();
        expect(result).assertTrue();
        console.log('Type_GetSuperClass_0100 done');
        done();
        console.log('Type_GetSuperClass_0100 end');
      })

    /*
     * @tc.number    : test_Type_IsAssignableFrom_0100
     * @tc.name      : test_Type_IsAssignableFrom_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("test_Type_IsAssignableFrom_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('test_Type_IsAssignableFrom_0100 start');
        let cls = new Foo();
        let result = cls.test_Type_IsAssignableFrom();
        expect(result).assertTrue();
        console.log('test_Type_IsAssignableFrom_0100 done');
        done();
        console.log('test_Type_IsAssignableFrom_0100 end');
      })

    /*
     * @tc.number    : FindModule_0100
     * @tc.name      : FindModule_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("FindModule_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('FindModule_0100 start');
        let cls = new Foo();
        let result = cls.test_FindModule();
        expect(result).assertTrue();
        console.log('FindModule_0100 done');
        done();
        console.log('FindModule_0100 end');
      })

    /*
     * @tc.number    : FindNamespace_0100
     * @tc.name      : FindNamespace_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("FindNamespace_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('FindNamespace_0100 start');
        let cls = new Foo();
        let result = cls.test_FindNamespace();
        expect(result).assertTrue();
        console.log('FindNamespace_0100 done');
        done();
        console.log('FindNamespace_0100 end');
      })

    /*
     * @tc.number    : FindEnum_0100
     * @tc.name      : FindEnum_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("FindEnum_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('FindEnum_0100 start');
        let cls = new Foo();
        let result = cls.test_FindEnum();
        expect(result).assertTrue();
        console.log('FindEnum_0100 done');
        done();
        console.log('FindEnum_0100 end');
      })
  })
}