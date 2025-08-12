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

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, TestType, Size, Level } from '@ohos/hypium'
import neuralnetworkcoretest from 'libneuralnetworkcoretest.so'

const OH_NN_INVALID_PARAMETER = 2;

export default function ActsNeuralNetworkRuntimeTypeTest2() {
  describe('ActsNeuralNetworkRuntimeTypeTest2', () => {

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSTOPK_0200
     * @tc.name    : NnrtOpTypeTopK002
     * @tc.desc    : test OH_NN_OPS_TOP_K
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeTopK002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeTopK002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_TOP_K", false);
        console.info("====>NnrtOpTypeTopK002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeTopK002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSARGMAX_0100
     * @tc.name    : NnrtOpTypeArgMax001
     * @tc.desc    : test OH_NN_OPS_ARG_MAX
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeArgMax001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeArgMax001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ARG_MAX", true);
        console.info("====>NnrtOpTypeArgMax001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeArgMax001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSARGMAX_0200
     * @tc.name    : NnrtOpTypeArgMax002
     * @tc.desc    : test OH_NN_OPS_ARG_MAX
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeArgMax002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeArgMax002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ARG_MAX", false);
        console.info("====>NnrtOpTypeArgMax002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeArgMax002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSUNSQUEEZE_0100
     * @tc.name    : NnrtOpTypeUnsqueeze001
     * @tc.desc    : test OH_NN_OPS_UNSQUEEZE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeUnsqueeze001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeUnsqueeze001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_UNSQUEEZE", true);
        console.info("====>NnrtOpTypeUnsqueeze001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeUnsqueeze001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSUNSQUEEZE_0200
     * @tc.name    : NnrtOpTypeUnsqueeze002
     * @tc.desc    : test OH_NN_OPS_UNSQUEEZE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeUnsqueeze002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeUnsqueeze002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_UNSQUEEZE", false);
        console.info("====>NnrtOpTypeUnsqueeze002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeUnsqueeze002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSGELU_0100
     * @tc.name    : NnrtOpTypeGelu001
     * @tc.desc    : test OH_NN_OPS_GELU
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeGelu001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeGelu001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_GELU", true);
        console.info("====>NnrtOpTypeGelu001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeGelu001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSGELU_0200
     * @tc.name    : NnrtOpTypeGelu002
     * @tc.desc    : test OH_NN_OPS_GELU
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeGelu002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeGelu002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_GELU", false);
        console.info("====>NnrtOpTypeGelu002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeGelu002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSUNSTACK_0100
     * @tc.name    : NnrtOpTypeUnstack001
     * @tc.desc    : test OH_NN_OPS_UNSTACK
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeUnstack001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeUnstack001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_UNSTACK", true);
        console.info("====>NnrtOpTypeUnstack001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeUnstack001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSUNSTACK_0200
     * @tc.name    : NnrtOpTypeUnstack002
     * @tc.desc    : test OH_NN_OPS_UNSTACK
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeUnstack002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeUnstack002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_UNSTACK", false);
        console.info("====>NnrtOpTypeUnstack002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeUnstack002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSABS_0100
     * @tc.name    : NnrtOpTypeAbs001
     * @tc.desc    : test OH_NN_OPS_ABS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeAbs001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeAbs001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ABS", true);
        console.info("====>NnrtOpTypeAbs001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeAbs001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSABS_0200
     * @tc.name    : NnrtOpTypeAbs002
     * @tc.desc    : test OH_NN_OPS_ABS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeAbs002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeAbs002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ABS", false);
        console.info("====>NnrtOpTypeAbs002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeAbs002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSERF_0100
     * @tc.name    : NnrtOpTypeErf001
     * @tc.desc    : test OH_NN_OPS_ERF
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeErf001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeErf001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ERF", true);
        console.info("====>NnrtOpTypeErf001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeErf001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSERF_0200
     * @tc.name    : NnrtOpTypeErf002
     * @tc.desc    : test OH_NN_OPS_ERF
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeErf002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeErf002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ERF", false);
        console.info("====>NnrtOpTypeErf002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeErf002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSEXP_0100
     * @tc.name    : NnrtOpTypeExp001
     * @tc.desc    : test OH_NN_OPS_EXP
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeExp001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeExp001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_EXP", true);
        console.info("====>NnrtOpTypeExp001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeExp001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSEXP_0200
     * @tc.name    : NnrtOpTypeExp002
     * @tc.desc    : test OH_NN_OPS_EXP
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeExp002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeExp002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_EXP", false);
        console.info("====>NnrtOpTypeExp002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeExp002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSLESS_0100
     * @tc.name    : NnrtOpTypeLess001
     * @tc.desc    : test OH_NN_OPS_LESS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeLess001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeLess001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_LESS", true);
        console.info("====>NnrtOpTypeLess001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeLess001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSLESS_0200
     * @tc.name    : NnrtOpTypeLess002
     * @tc.desc    : test OH_NN_OPS_LESS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeLess002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeLess002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_LESS", false);
        console.info("====>NnrtOpTypeLess002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeLess002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSSELECT_0100
     * @tc.name    : NnrtOpTypeSelect001
     * @tc.desc    : test OH_NN_OPS_SELECT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeSelect001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeSelect001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_SELECT", true);
        console.info("====>NnrtOpTypeSelect001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeSelect001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSSELECT_0200
     * @tc.name    : NnrtOpTypeSelect002
     * @tc.desc    : test OH_NN_OPS_SELECT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeSelect002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeSelect002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_SELECT", false);
        console.info("====>NnrtOpTypeSelect002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeSelect002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSSQUARE_0100
     * @tc.name    : NnrtOpTypeSquare001
     * @tc.desc    : test OH_NN_OPS_SQUARE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeSquare001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeSquare001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_SQUARE", true);
        console.info("====>NnrtOpTypeSquare001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeSquare001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSSQUARE_0200
     * @tc.name    : NnrtOpTypeSquare002
     * @tc.desc    : test OH_NN_OPS_SQUARE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeSquare002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeSquare002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_SQUARE", false);
        console.info("====>NnrtOpTypeSquare002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeSquare002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSFLATTEN_0100
     * @tc.name    : NnrtOpTypeFlatten001
     * @tc.desc    : test OH_NN_OPS_FLATTEN
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeFlatten001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeFlatten001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_FLATTEN", true);
        console.info("====>NnrtOpTypeFlatten001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeFlatten001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSFLATTEN_0200
     * @tc.name    : NnrtOpTypeFlatten002
     * @tc.desc    : test OH_NN_OPS_FLATTEN
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeFlatten002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeFlatten002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_FLATTEN", false);
        console.info("====>NnrtOpTypeFlatten002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeFlatten002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSDEPTHTOSPACE_0100
     * @tc.name    : NnrtOpTypeDepthToSpace001
     * @tc.desc    : test OH_NN_OPS_DEPTH_TO_SPACE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeDepthToSpace001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeDepthToSpace001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_DEPTH_TO_SPACE", true);
        console.info("====>NnrtOpTypeDepthToSpace001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeDepthToSpace001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSDEPTHTOSPACE_0200
     * @tc.name    : NnrtOpTypeDepthToSpace002
     * @tc.desc    : test OH_NN_OPS_DEPTH_TO_SPACE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeDepthToSpace002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeDepthToSpace002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_DEPTH_TO_SPACE", false);
        console.info("====>NnrtOpTypeDepthToSpace002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeDepthToSpace002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSRANGE_0100
     * @tc.name    : NnrtOpTypeRange001
     * @tc.desc    : test OH_NN_OPS_RANGE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeRange001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeRange001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_RANGE", true);
        console.info("====>NnrtOpTypeRange001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeRange001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSRANGE_0200
     * @tc.name    : NnrtOpTypeRange002
     * @tc.desc    : test OH_NN_OPS_RANGE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeRange002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeRange002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_RANGE", false);
        console.info("====>NnrtOpTypeRange002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeRange002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSINSTANCENORM_0100
     * @tc.name    : NnrtOpTypeInstanceNorm001
     * @tc.desc    : test OH_NN_OPS_INSTANCE_NORM
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeInstanceNorm001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeInstanceNorm001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_INSTANCE_NORM", true);
        console.info("====>NnrtOpTypeInstanceNorm001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeInstanceNorm001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSINSTANCENORM_0200
     * @tc.name    : NnrtOpTypeInstanceNorm002
     * @tc.desc    : test OH_NN_OPS_INSTANCE_NORM
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeInstanceNorm002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeInstanceNorm002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_INSTANCE_NORM", false);
        console.info("====>NnrtOpTypeInstanceNorm002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeInstanceNorm002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSCONSTANTOFSHAPE_0100
     * @tc.name    : NnrtOpTypeConstantOfShape001
     * @tc.desc    : test OH_NN_OPS_CONSTANT_OF_SHAPE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeConstantOfShape001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeConstantOfShape001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_CONSTANT_OF_SHAPE", true);
        console.info("====>NnrtOpTypeConstantOfShape001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeConstantOfShape001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSCONSTANTOFSHAPE_0200
     * @tc.name    : NnrtOpTypeConstantOfShape002
     * @tc.desc    : test OH_NN_OPS_CONSTANT_OF_SHAPE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeConstantOfShape002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeConstantOfShape002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_CONSTANT_OF_SHAPE", false);
        console.info("====>NnrtOpTypeConstantOfShape002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeConstantOfShape002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSBROADCASTTO_0100
     * @tc.name    : NnrtOpTypeBroadcastTo001
     * @tc.desc    : test OH_NN_OPS_BROADCAST_TO
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeBroadcastTo001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeBroadcastTo001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_BROADCAST_TO", true);
        console.info("====>NnrtOpTypeBroadcastTo001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeBroadcastTo001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSBROADCASTTO_0200
     * @tc.name    : NnrtOpTypeBroadcastTo002
     * @tc.desc    : test OH_NN_OPS_BROADCAST_TO
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeBroadcastTo002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeBroadcastTo002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_BROADCAST_TO", false);
        console.info("====>NnrtOpTypeBroadcastTo002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeBroadcastTo002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSEQUAL_0100
     * @tc.name    : NnrtOpTypeEqual001
     * @tc.desc    : test OH_NN_OPS_EQUAL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeEqual001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeEqual001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_EQUAL", true);
        console.info("====>NnrtOpTypeEqual001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeEqual001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSEQUAL_0200
     * @tc.name    : NnrtOpTypeEqual002
     * @tc.desc    : test OH_NN_OPS_EQUAL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeEqual002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeEqual002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_EQUAL", false);
        console.info("====>NnrtOpTypeEqual002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeEqual002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSGREATER_0100
     * @tc.name    : NnrtOpTypeGreater001
     * @tc.desc    : test OH_NN_OPS_GREATER
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeGreater001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeGreater001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_GREATER", true);
        console.info("====>NnrtOpTypeGreater001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeGreater001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSGREATER_0200
     * @tc.name    : NnrtOpTypeGreater002
     * @tc.desc    : test OH_NN_OPS_GREATER
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeGreater002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeGreater002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_GREATER", false);
        console.info("====>NnrtOpTypeGreater002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeGreater002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSNOTEQUAL_0100
     * @tc.name    : NnrtOpTypeNotEqual001
     * @tc.desc    : test OH_NN_OPS_NOT_EQUAL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeNotEqual001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeNotEqual001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_NOT_EQUAL", true);
        console.info("====>NnrtOpTypeNotEqual001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeNotEqual001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSNOTEQUAL_0200
     * @tc.name    : NnrtOpTypeNotEqual002
     * @tc.desc    : test OH_NN_OPS_NOT_EQUAL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeNotEqual002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeNotEqual002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_NOT_EQUAL", false);
        console.info("====>NnrtOpTypeNotEqual002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeNotEqual002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSGREATEREQUAL_0100
     * @tc.name    : NnrtOpTypeGreaterEqual001
     * @tc.desc    : test OH_NN_OPS_GREATER_EQUAL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeGreaterEqual001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeGreaterEqual001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_GREATER_EQUAL", true);
        console.info("====>NnrtOpTypeGreaterEqual001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeGreaterEqual001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSGREATEREQUAL_0200
     * @tc.name    : NnrtOpTypeGreaterEqual002
     * @tc.desc    : test OH_NN_OPS_GREATER_EQUAL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeGreaterEqual002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeGreaterEqual002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_GREATER_EQUAL", false);
        console.info("====>NnrtOpTypeGreaterEqual002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeGreaterEqual002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSLEAKYRELU_0100
     * @tc.name    : NnrtOpTypeLeakyRelu001
     * @tc.desc    : test OH_NN_OPS_LEAKY_RELU
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeLeakyRelu001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeLeakyRelu001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_LEAKY_RELU", true);
        console.info("====>NnrtOpTypeLeakyRelu001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeLeakyRelu001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSLEAKYRELU_0200
     * @tc.name    : NnrtOpTypeLeakyRelu002
     * @tc.desc    : test OH_NN_OPS_LEAKY_RELU
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeLeakyRelu002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeLeakyRelu002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_LEAKY_RELU", false);
        console.info("====>NnrtOpTypeLeakyRelu002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeLeakyRelu002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSLSTM_0100
     * @tc.name    : NnrtOpTypeLstm001
     * @tc.desc    : test OH_NN_OPS_LSTM
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeLstm001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeLstm001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_LSTM", true);
        console.info("====>NnrtOpTypeLstm001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeLstm001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSLSTM_0200
     * @tc.name    : NnrtOpTypeLstm002
     * @tc.desc    : test OH_NN_OPS_LSTM
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeLstm002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeLstm002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_LSTM", false);
        console.info("====>NnrtOpTypeLstm002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeLstm002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSCLIP_0100
     * @tc.name    : NnrtOpTypeClip001
     * @tc.desc    : test OH_NN_OPS_CLIP
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeClip001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeClip001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_CLIP", true);
        console.info("====>NnrtOpTypeClip001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeClip001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSCLIP_0200
     * @tc.name    : NnrtOpTypeClip002
     * @tc.desc    : test OH_NN_OPS_CLIP
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeClip002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeClip002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_CLIP", false);
        console.info("====>NnrtOpTypeClip002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeClip002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSALL_0100
     * @tc.name    : NnrtOpTypeAll001
     * @tc.desc    : test OH_NN_OPS_ALL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeAll001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeAll001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ALL", true);
        console.info("====>NnrtOpTypeAll001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeAll001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSALL_0200
     * @tc.name    : NnrtOpTypeAll002
     * @tc.desc    : test OH_NN_OPS_ALL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeAll002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeAll002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ALL", false);
        console.info("====>NnrtOpTypeAll002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeAll002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSASSERT_0100
     * @tc.name    : NnrtOpTypeAssert001
     * @tc.desc    : test OH_NN_OPS_ASSERT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeAssert001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeAssert001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ASSERT", true);
        console.info("====>NnrtOpTypeAssert001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeAssert001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSASSERT_0200
     * @tc.name    : NnrtOpTypeAssert002
     * @tc.desc    : test OH_NN_OPS_ASSERT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeAssert002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeAssert002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_ASSERT", false);
        console.info("====>NnrtOpTypeAssert002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeAssert002 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSCOS_0100
     * @tc.name    : NnrtOpTypeCos001
     * @tc.desc    : test OH_NN_OPS_COS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeCos001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeCos001 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_COS", true);
        console.info("====>NnrtOpTypeCos001 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeCos001 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_NEURALNETWORKRUNTIME_OPERATIONTYPEOPSCOS_0200
     * @tc.name    : NnrtOpTypeCos002
     * @tc.desc    : test OH_NN_OPS_COS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('NnrtOpTypeCos002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>NnrtOpTypeCos002 start====");
        let result : number = neuralnetworkcoretest.NNOperationTypeCommonTest("OH_NN_OPS_COS", false);
        console.info("====>NnrtOpTypeCos002 result: " + result);
        expect(result).assertEqual(OH_NN_INVALID_PARAMETER);
        done()
      } catch (err) {
        console.error("====>NnrtOpTypeCos002 catch err: " + err);
      }
    })
  })
}
