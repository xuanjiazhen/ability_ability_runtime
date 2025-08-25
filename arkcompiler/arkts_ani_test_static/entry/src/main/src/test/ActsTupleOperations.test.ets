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

export default function tupleOperationsTest() {

  describe("ActsTupleOperationsTest", (): void => {
    try {
      loadLibrary("entry")
    } catch(e: Error) {
      console.log('loadFailed', e.message);
    }

    /*
     * @tc.number    : TupleValue_GetItem_Boolean_0100
     * @tc.name      : TupleValue_GetItem_Boolean_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Boolean_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Boolean_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Boolean();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Boolean_0100 done');
        done();
        console.log('TupleValue_GetItem_Boolean_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Char_0100
     * @tc.name      : TupleValue_GetItem_Char_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Char_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Char_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Char();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Char_0100 done');
        done();
        console.log('TupleValue_GetItem_Char_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Byte_0100
     * @tc.name      : TupleValue_GetItem_Byte_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Byte_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Byte_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Byte();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Byte_0100 done');
        done();
        console.log('TupleValue_GetItem_Byte_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Short_0100
     * @tc.name      : TupleValue_GetItem_Short_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Short_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Short_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Short();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Short_0100 done');
        done();
        console.log('TupleValue_GetItem_Short_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Int_0100
     * @tc.name      : TupleValue_GetItem_Int_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Int_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Int_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Int();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Int_0100 done');
        done();
        console.log('TupleValue_GetItem_Int_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Long_0100
     * @tc.name      : TupleValue_GetItem_Long_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Long_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Long_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Long();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Long_0100 done');
        done();
        console.log('TupleValue_GetItem_Long_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Float_0100
     * @tc.name      : TupleValue_GetItem_Float_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Float_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Float_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Float();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Float_0100 done');
        done();
        console.log('TupleValue_GetItem_Float_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Double_0100
     * @tc.name      : TupleValue_GetItem_Double_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Double_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Double_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Double();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Double_0100 done');
        done();
        console.log('TupleValue_GetItem_Double_0100 end');
      })

    /*
     * @tc.number    : TupleValue_GetItem_Ref_0100
     * @tc.name      : TupleValue_GetItem_Ref_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetItem_Ref_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetItem_Ref_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetItem_Ref();
        expect(result).assertTrue();
        console.log('TupleValue_GetItem_Ref_0100 done');
        done();
        console.log('TupleValue_GetItem_Ref_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Boolean_0100
     * @tc.name      : TupleValue_SetItem_Boolean_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Boolean_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Boolean_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Boolean();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Boolean_0100 done');
        done();
        console.log('TupleValue_SetItem_Boolean_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Char_0100
     * @tc.name      : TupleValue_SetItem_Char_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Char_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Char_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Char();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Char_0100 done');
        done();
        console.log('TupleValue_SetItem_Char_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Byte_0100
     * @tc.name      : TupleValue_SetItem_Byte_0100
     * @tc.desc      : Medium极Test
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Byte_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Byte_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Byte();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Byte_0100 done');
        done();
        console.log('TupleValue_SetItem_Byte_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Short_0100
     * @tc.name      : TupleValue_SetItem_Short_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Short_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Short_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Short();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Short_0100 done');
        done();
        console.log('TupleValue_SetItem_Short_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Int_0100
     * @tc.name      : TupleValue_SetItem_Int_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Int_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Int_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Int();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Int_0100 done');
        done();
        console.log('TupleValue_SetItem_Int_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Long_0100
     * @tc.name      : TupleValue_SetItem_Long_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Long_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Long_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Long();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Long_0100 done');
        done();
        console.log('TupleValue_SetItem_Long_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Float_0100
     * @tc.name      : TupleValue_SetItem_Float_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Float_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Float_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Float();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Float_0100 done');
        done();
        console.log('TupleValue_SetItem_Float_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Double_0100
     * @tc.name      : TupleValue_SetItem_Double_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Double_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Double_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Double();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Double_0100 done');
        done();
        console.log('TupleValue_SetItem_Double_0100 end');
      })

    /*
     * @tc.number    : TupleValue_SetItem_Ref_0100
     * @tc.name      : TupleValue_SetItem_Ref_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_SetItem_Ref_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_SetItem_Ref_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_SetItem_Ref();
        expect(result).assertTrue();
        console.log('TupleValue_SetItem_Ref_0100 done');
        done();
        console.log('TupleValue_SetItem_Ref_0100 end');
      })


    /*
     * @tc.number    : TupleValue_GetNumberOfItems_0100
     * @tc.name      : TupleValue_GetNumberOfItems_0100
     * @tc.desc      : MediumTest
     * @tc.size      : Function
     * @tc.type      : Function test
     * @tc.level     : Level 0
     */
    it("TupleValue_GetNumberOfItems_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done: () => void): Promise<void> => {
        console.log('TupleValue_GetNumberOfItems_0100 start');
        let cls = new Foo();
        let result = cls.test_TupleValue_GetNumberOfItems();
        expect(result).assertTrue();
        console.log('TupleValue_GetNumberOfItems_0100 done');
        done();
        console.log('TupleValue_GetNumberOfItems_0100 end');
      })
  })
}