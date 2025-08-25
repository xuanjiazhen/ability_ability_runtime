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

export default function enumOperationsTest() {

  describe("ActsEnumOperationsTest", (): void => {
    try {
      loadLibrary("entry")
    } catch(e: Error) {
      console.log('loadFailed', e.message);
    }

    /*
     * @tc.number    : Enum_GetEnumItemByName_0100
     * @tc.name      : Enum_GetEnumItemByName_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Enum_GetEnumItemByName_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Enum_GetEnumItemByName_0100 start');
        let cls = new Foo();
        let result = cls.test_Enum_GetEnumItemByName();
        expect(result).assertTrue();
        console.log('Enum_GetEnumItemByName_0100 done');
        done();
        console.log('Enum_GetEnumItemByName_0100 end');
      })

    /*
     * @tc.number    : Enum_GetEnumItemByIndex_0100
     * @tc.name      : Enum_GetEnumItemByIndex_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("Enum_GetEnumItemByIndex_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('Enum_GetEnumItemByIndex_0100 start');
        let cls = new Foo();
        let result = cls.test_Enum_GetEnumItemByIndex();
        expect(result).assertTrue();
        console.log('Enum_GetEnumItemByIndex_0100 done');
        done();
        console.log('Enum_GetEnumItemByIndex_0100 end');
      })

    /*
     * @tc.number    : EnumItem_GetEnum_0100
     * @tc.name      : EnumItem_GetEnum_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("EnumItem_GetEnum_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('EnumItem_GetEnum_0100 start');
        let cls = new Foo();
        let result = cls.test_EnumItem_GetEnum();
        expect(result).assertTrue();
        console.log('EnumItem_GetEnum_0100 done');
        done();
        console.log('EnumItem_GetEnum_0100 end');
      })

    /*
     * @tc.number    : EnumItem_GetValue_Int_0100
     * @tc.name      : EnumItem_GetValue_Int_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("EnumItem_GetValue_Int_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('EnumItem_GetValue_Int_0100 start');
        let cls = new Foo();
        let result = cls.test_EnumItem_GetValue_Int();
        expect(result).assertTrue();
        console.log('EnumItem_GetValue_Int_0100 done');
        done();
        console.log('EnumItem_GetValue_Int_0100 end');
      })

    /*
     * @tc.number    : EnumItem_GetValue_String_0100
     * @tc.name      : EnumItem_GetValue_String_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("EnumItem_GetValue_String_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('EnumItem_GetValue_String_0100 start');
        let cls = new Foo();
        let result = cls.test_EnumItem_GetValue_String();
        expect(result).assertTrue();
        console.log('EnumItem_GetValue_String_0100 done');
        done();
        console.log('EnumItem_GetValue_String_0100 end');
      })

    /*
     * @tc.number    : EnumItem_GetName_0100
     * @tc.name      : EnumItem_GetName_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("EnumItem_GetName_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('EnumItem_GetName_极0100 start');
        let cls = new Foo();
        let result = cls.test_EnumItem_GetName();
        expect(result).assertTrue();
        console.log('EnumItem_GetName_0100 done');
        done();
        console.log('EnumItem_GetName_0100 end');
      })

    /*
     * @tc.number    : EnumItem_GetIndex_0100
     * @tc.name      : EnumItem_GetIndex_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("EnumItem_GetIndex_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('EnumItem_GetIndex_0100 start');
        let cls = new Foo();
        let result = cls.test_EnumItem_GetIndex();
        expect(result).assertTrue();
        console.log('EnumItem_GetIndex_0100 done');
        done();
        console.log('EnumItem_GetIndex_0100 end');
      })
  })
}