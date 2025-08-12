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

#include "NeuralNetworkCoreTest.h"
#include "napi/native_api.h"
#include <js_native_api_types.h>
#include <napi/native_api.h>
#include <neural_network_runtime/neural_network_core.h>
#include <neural_network_runtime/neural_network_runtime.h>

namespace {
static const uint32_t MAX_NAME_LENGTH = 1024;
}

NN_TensorDesc *createTensorDesc(const int32_t *shape, size_t shapeNum, OH_NN_DataType dataType, OH_NN_Format format)
{
    NN_TensorDesc *tensorDescTmp = OH_NNTensorDesc_Create();
    if (tensorDescTmp == nullptr) {
        return nullptr;
    }

    OH_NN_ReturnCode ret = OH_NNTensorDesc_SetDataType(tensorDescTmp, dataType);
    if (ret != OH_NN_SUCCESS) {
        return nullptr;
    }

    if (shape != nullptr) {
        ret = OH_NNTensorDesc_SetShape(tensorDescTmp, shape, shapeNum);
        if (ret != OH_NN_SUCCESS) {
            return nullptr;
        }
    }

    ret = OH_NNTensorDesc_SetFormat(tensorDescTmp, format);
    if (ret != OH_NN_SUCCESS) {
        return nullptr;
    }

    return tensorDescTmp;
}

napi_value HandleTensorOperations(napi_env env, napi_callback_info info)
{
    // 解析输入参数获取操作类型
    size_t argc = 1;
    napi_value args[1];
    OH_NN_TensorType opType = OH_NN_TensorType::OH_NN_TENSOR; // 默认类型

    // 获取传入的参数
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status == napi_ok && argc >= 1) {
        int32_t opTypeInt;
        if (napi_get_value_int32(env, args[0], &opTypeInt) == napi_ok) {
            // 检查范围并设置对应的操作类型
            if (opTypeInt >= 0 && opTypeInt <= 3) {
                opType = static_cast<OH_NN_TensorType>(opTypeInt);
            }
        }
    }

    // 根据操作类型确定第三个参数值
    OH_NN_TensorType tensorType;
    switch (opType) {
    case OH_NN_TENSOR:
        tensorType = OH_NN_TENSOR;
        break;
    case OH_NN_ADD_ACTIVATIONTYPE:
        tensorType = OH_NN_ADD_ACTIVATIONTYPE;
        break;
    case OH_NN_AVG_POOL_KERNEL_SIZE:
        tensorType = OH_NN_AVG_POOL_KERNEL_SIZE;
        break;
    case OH_NN_AVG_POOL_STRIDE:
        tensorType = OH_NN_AVG_POOL_STRIDE;
        break;
    case OH_NN_AVG_POOL_PAD_MODE:
        tensorType = OH_NN_AVG_POOL_PAD_MODE;
        break;
    case OH_NN_AVG_POOL_PAD:
        tensorType = OH_NN_AVG_POOL_PAD;
        break;
    case OH_NN_AVG_POOL_ACTIVATION_TYPE:
        tensorType = OH_NN_AVG_POOL_ACTIVATION_TYPE;
        break;
    case OH_NN_BATCH_NORM_EPSILON:
        tensorType = OH_NN_BATCH_NORM_EPSILON;
        break;
    case OH_NN_BATCH_TO_SPACE_ND_BLOCKSIZE:
        tensorType = OH_NN_BATCH_TO_SPACE_ND_BLOCKSIZE;
        break;
    case OH_NN_BATCH_TO_SPACE_ND_CROPS:
        tensorType = OH_NN_BATCH_TO_SPACE_ND_CROPS;
        break;
    case OH_NN_CONCAT_AXIS:
        tensorType = OH_NN_CONCAT_AXIS;
        break;
    case OH_NN_CONV2D_STRIDES:
        tensorType = OH_NN_CONV2D_STRIDES;
        break;
    case OH_NN_CONV2D_PAD:
        tensorType = OH_NN_CONV2D_PAD;
        break;
    case OH_NN_CONV2D_DILATION:
        tensorType = OH_NN_CONV2D_DILATION;
        break;
    case OH_NN_CONV2D_PAD_MODE:
        tensorType = OH_NN_CONV2D_PAD_MODE;
        break;
    case OH_NN_CONV2D_ACTIVATION_TYPE:
        tensorType = OH_NN_CONV2D_ACTIVATION_TYPE;
        break;
    case OH_NN_CONV2D_GROUP:
        tensorType = OH_NN_CONV2D_GROUP;
        break;
    case OH_NN_CONV2D_TRANSPOSE_STRIDES:
        tensorType = OH_NN_CONV2D_TRANSPOSE_STRIDES;
        break;
    case OH_NN_CONV2D_TRANSPOSE_PAD:
        tensorType = OH_NN_CONV2D_TRANSPOSE_PAD;
        break;
    case OH_NN_CONV2D_TRANSPOSE_DILATION:
        tensorType = OH_NN_CONV2D_TRANSPOSE_DILATION;
        break;
    case OH_NN_CONV2D_TRANSPOSE_OUTPUT_PADDINGS:
        tensorType = OH_NN_CONV2D_TRANSPOSE_OUTPUT_PADDINGS;
        break;
    case OH_NN_CONV2D_TRANSPOSE_PAD_MODE:
        tensorType = OH_NN_CONV2D_TRANSPOSE_PAD_MODE;
        break;
    case OH_NN_CONV2D_TRANSPOSE_ACTIVATION_TYPE:
        tensorType = OH_NN_CONV2D_TRANSPOSE_ACTIVATION_TYPE;
        break;
    case OH_NN_CONV2D_TRANSPOSE_GROUP:
        tensorType = OH_NN_CONV2D_TRANSPOSE_GROUP;
        break;
    case OH_NN_DEPTHWISE_CONV2D_NATIVE_STRIDES:
        tensorType = OH_NN_DEPTHWISE_CONV2D_NATIVE_STRIDES;
        break;
    case OH_NN_DEPTHWISE_CONV2D_NATIVE_PAD:
        tensorType = OH_NN_DEPTHWISE_CONV2D_NATIVE_PAD;
        break;
    case OH_NN_DEPTHWISE_CONV2D_NATIVE_DILATION:
        tensorType = OH_NN_DEPTHWISE_CONV2D_NATIVE_DILATION;
        break;
    case OH_NN_DEPTHWISE_CONV2D_NATIVE_PAD_MODE:
        tensorType = OH_NN_DEPTHWISE_CONV2D_NATIVE_PAD_MODE;
        break;
    case OH_NN_DEPTHWISE_CONV2D_NATIVE_ACTIVATION_TYPE:
        tensorType = OH_NN_DEPTHWISE_CONV2D_NATIVE_ACTIVATION_TYPE;
        break;
    case OH_NN_DIV_ACTIVATIONTYPE:
        tensorType = OH_NN_DIV_ACTIVATIONTYPE;
        break;
    case OH_NN_ELTWISE_MODE:
        tensorType = OH_NN_ELTWISE_MODE;
        break;
    case OH_NN_FULL_CONNECTION_AXIS:
        tensorType = OH_NN_FULL_CONNECTION_AXIS;
        break;
    case OH_NN_FULL_CONNECTION_ACTIVATIONTYPE:
        tensorType = OH_NN_FULL_CONNECTION_ACTIVATIONTYPE;
        break;
    case OH_NN_MATMUL_TRANSPOSE_A:
        tensorType = OH_NN_MATMUL_TRANSPOSE_A;
        break;
    case OH_NN_MATMUL_TRANSPOSE_B:
        tensorType = OH_NN_MATMUL_TRANSPOSE_B;
        break;
    case OH_NN_MATMUL_ACTIVATION_TYPE:
        tensorType = OH_NN_MATMUL_ACTIVATION_TYPE;
        break;
    case OH_NN_MAX_POOL_KERNEL_SIZE:
        tensorType = OH_NN_MAX_POOL_KERNEL_SIZE;
        break;
    case OH_NN_MAX_POOL_STRIDE:
        tensorType = OH_NN_MAX_POOL_STRIDE;
        break;
    case OH_NN_MAX_POOL_PAD_MODE:
        tensorType = OH_NN_MAX_POOL_PAD_MODE;
        break;
    case OH_NN_MAX_POOL_PAD:
        tensorType = OH_NN_MAX_POOL_PAD;
        break;
    case OH_NN_MAX_POOL_ACTIVATION_TYPE:
        tensorType = OH_NN_MAX_POOL_ACTIVATION_TYPE;
        break;
    case OH_NN_MUL_ACTIVATION_TYPE:
        tensorType = OH_NN_MUL_ACTIVATION_TYPE;
        break;
    case OH_NN_ONE_HOT_AXIS:
        tensorType = OH_NN_ONE_HOT_AXIS;
        break;
    case OH_NN_PAD_CONSTANT_VALUE:
        tensorType = OH_NN_PAD_CONSTANT_VALUE;
        break;
    case OH_NN_SCALE_ACTIVATIONTYPE:
        tensorType = OH_NN_SCALE_ACTIVATIONTYPE;
        break;
    case OH_NN_SCALE_AXIS:
        tensorType = OH_NN_SCALE_AXIS;
        break;
    case OH_NN_SOFTMAX_AXIS:
        tensorType = OH_NN_SOFTMAX_AXIS;
        break;
    case OH_NN_SPACE_TO_BATCH_ND_BLOCK_SHAPE:
        tensorType = OH_NN_SPACE_TO_BATCH_ND_BLOCK_SHAPE;
        break;
    case OH_NN_SPACE_TO_BATCH_ND_PADDINGS:
        tensorType = OH_NN_SPACE_TO_BATCH_ND_PADDINGS;
        break;
    case OH_NN_SPLIT_AXIS:
        tensorType = OH_NN_SPLIT_AXIS;
        break;
    case OH_NN_SPLIT_OUTPUT_NUM:
        tensorType = OH_NN_SPLIT_OUTPUT_NUM;
        break;
    case OH_NN_SPLIT_SIZE_SPLITS:
        tensorType = OH_NN_SPLIT_SIZE_SPLITS;
        break;
    case OH_NN_SQUEEZE_AXIS:
        tensorType = OH_NN_SQUEEZE_AXIS;
        break;
    case OH_NN_STACK_AXIS:
        tensorType = OH_NN_STACK_AXIS;
        break;
    case OH_NN_STRIDED_SLICE_BEGIN_MASK:
        tensorType = OH_NN_STRIDED_SLICE_BEGIN_MASK;
        break;
    case OH_NN_STRIDED_SLICE_END_MASK:
        tensorType = OH_NN_STRIDED_SLICE_END_MASK;
        break;
    case OH_NN_STRIDED_SLICE_ELLIPSIS_MASK:
        tensorType = OH_NN_STRIDED_SLICE_ELLIPSIS_MASK;
        break;
    case OH_NN_STRIDED_SLICE_NEW_AXIS_MASK:
        tensorType = OH_NN_STRIDED_SLICE_NEW_AXIS_MASK;
        break;
    case OH_NN_STRIDED_SLICE_SHRINK_AXIS_MASK:
        tensorType = OH_NN_STRIDED_SLICE_SHRINK_AXIS_MASK;
        break;
    case OH_NN_SUB_ACTIVATIONTYPE:
        tensorType = OH_NN_SUB_ACTIVATIONTYPE;
        break;
    case OH_NN_REDUCE_MEAN_KEEP_DIMS:
        tensorType = OH_NN_REDUCE_MEAN_KEEP_DIMS;
        break;
    case OH_NN_RESIZE_BILINEAR_NEW_HEIGHT:
        tensorType = OH_NN_RESIZE_BILINEAR_NEW_HEIGHT;
        break;
    case OH_NN_RESIZE_BILINEAR_NEW_WIDTH:
        tensorType = OH_NN_RESIZE_BILINEAR_NEW_WIDTH;
        break;
    case OH_NN_RESIZE_BILINEAR_PRESERVE_ASPECT_RATIO:
        tensorType = OH_NN_RESIZE_BILINEAR_PRESERVE_ASPECT_RATIO;
        break;
    case OH_NN_RESIZE_BILINEAR_COORDINATE_TRANSFORM_MODE:
        tensorType = OH_NN_RESIZE_BILINEAR_COORDINATE_TRANSFORM_MODE;
        break;
    case OH_NN_RESIZE_BILINEAR_EXCLUDE_OUTSIDE:
        tensorType = OH_NN_RESIZE_BILINEAR_EXCLUDE_OUTSIDE;
        break;
    case OH_NN_LAYER_NORM_BEGIN_NORM_AXIS:
        tensorType = OH_NN_LAYER_NORM_BEGIN_NORM_AXIS;
        break;
    case OH_NN_LAYER_NORM_EPSILON:
        tensorType = OH_NN_LAYER_NORM_EPSILON;
        break;
    case OH_NN_LAYER_NORM_BEGIN_PARAM_AXIS:
        tensorType = OH_NN_LAYER_NORM_BEGIN_PARAM_AXIS;
        break;
    case OH_NN_LAYER_NORM_ELEMENTWISE_AFFINE:
        tensorType = OH_NN_LAYER_NORM_ELEMENTWISE_AFFINE;
        break;
    case OH_NN_REDUCE_PROD_KEEP_DIMS:
        tensorType = OH_NN_REDUCE_PROD_KEEP_DIMS;
        break;
    case OH_NN_REDUCE_ALL_KEEP_DIMS:
        tensorType = OH_NN_REDUCE_ALL_KEEP_DIMS;
        break;
    case OH_NN_QUANT_DTYPE_CAST_SRC_T:
        tensorType = OH_NN_QUANT_DTYPE_CAST_SRC_T;
        break;
    case OH_NN_QUANT_DTYPE_CAST_DST_T:
        tensorType = OH_NN_QUANT_DTYPE_CAST_DST_T;
        break;
    case OH_NN_TOP_K_SORTED:
        tensorType = OH_NN_TOP_K_SORTED;
        break;
    case OH_NN_ARG_MAX_AXIS:
        tensorType = OH_NN_ARG_MAX_AXIS;
        break;
    case OH_NN_ARG_MAX_KEEPDIMS:
        tensorType = OH_NN_ARG_MAX_KEEPDIMS;
        break;
    case OH_NN_UNSQUEEZE_AXIS:
        tensorType = OH_NN_UNSQUEEZE_AXIS;
        break;
    case OH_NN_UNSTACK_AXIS:
        tensorType = OH_NN_UNSTACK_AXIS;
        break;
    case OH_NN_FLATTEN_AXIS:
        tensorType = OH_NN_FLATTEN_AXIS;
        break;
    case OH_NN_DEPTH_TO_SPACE_BLOCK_SIZE:
        tensorType = OH_NN_DEPTH_TO_SPACE_BLOCK_SIZE;
        break;
    case OH_NN_DEPTH_TO_SPACE_MODE:
        tensorType = OH_NN_DEPTH_TO_SPACE_MODE;
        break;
    case OH_NN_RANGE_START:
        tensorType = OH_NN_RANGE_START;
        break;
    case OH_NN_RANGE_LIMIT:
        tensorType = OH_NN_RANGE_LIMIT;
        break;
    case OH_NN_RANGE_DELTA:
        tensorType = OH_NN_RANGE_DELTA;
        break;
    case OH_NN_CONSTANT_OF_SHAPE_DATA_TYPE:
        tensorType = OH_NN_CONSTANT_OF_SHAPE_DATA_TYPE;
        break;
    case OH_NN_CONSTANT_OF_SHAPE_VALUE:
        tensorType = OH_NN_CONSTANT_OF_SHAPE_VALUE;
        break;
    case OH_NN_BROADCAST_TO_SHAPE:
        tensorType = OH_NN_BROADCAST_TO_SHAPE;
        break;
    case OH_NN_INSTANCE_NORM_EPSILON:
        tensorType = OH_NN_INSTANCE_NORM_EPSILON;
        break;
    case OH_NN_EXP_BASE:
        tensorType = OH_NN_EXP_BASE;
        break;
    case OH_NN_EXP_SCALE:
        tensorType = OH_NN_EXP_SCALE;
        break;
    case OH_NN_EXP_SHIFT:
        tensorType = OH_NN_EXP_SHIFT;
        break;
    case OH_NN_LEAKY_RELU_NEGATIVE_SLOPE:
        tensorType = OH_NN_LEAKY_RELU_NEGATIVE_SLOPE;
        break;
    case OH_NN_LSTM_BIDIRECTIONAL:
        tensorType = OH_NN_LSTM_BIDIRECTIONAL;
        break;
    case OH_NN_LSTM_HAS_BIAS:
        tensorType = OH_NN_LSTM_HAS_BIAS;
        break;
    case OH_NN_LSTM_INPUT_SIZE:
        tensorType = OH_NN_LSTM_INPUT_SIZE;
        break;
    case OH_NN_LSTM_HIDDEN_SIZE:
        tensorType = OH_NN_LSTM_HIDDEN_SIZE;
        break;
    case OH_NN_LSTM_NUM_LAYERS:
        tensorType = OH_NN_LSTM_NUM_LAYERS;
        break;
    case OH_NN_LSTM_NUM_DIRECTIONS:
        tensorType = OH_NN_LSTM_NUM_DIRECTIONS;
        break;
    case OH_NN_LSTM_DROPOUT:
        tensorType = OH_NN_LSTM_DROPOUT;
        break;
    case OH_NN_LSTM_ZONEOUT_CELL:
        tensorType = OH_NN_LSTM_ZONEOUT_CELL;
        break;
    case OH_NN_LSTM_ZONEOUT_HIDDEN:
        tensorType = OH_NN_LSTM_ZONEOUT_HIDDEN;
        break;
    case OH_NN_LSTM_PROJ_SIZE:
        tensorType = OH_NN_LSTM_PROJ_SIZE;
        break;
    case OH_NN_CLIP_MAX:
        tensorType = OH_NN_CLIP_MAX;
        break;
    case OH_NN_CLIP_MIN:
        tensorType = OH_NN_CLIP_MIN;
        break;
    case OH_NN_ALL_KEEP_DIMS:
        tensorType = OH_NN_ALL_KEEP_DIMS;
        break;
    case OH_NN_ASSERT_SUMMARIZE:
        tensorType = OH_NN_ASSERT_SUMMARIZE;
        break;
    case OH_NN_POW_SCALE:
        tensorType = OH_NN_POW_SCALE;
        break;
    case OH_NN_POW_SHIFT:
        tensorType = OH_NN_POW_SHIFT;
        break;
    case OH_NN_AVG_POOL_ROUND_MODE:
        tensorType = OH_NN_AVG_POOL_ROUND_MODE;
        break;
    case OH_NN_AVG_POOL_GLOBAL:
        tensorType = OH_NN_AVG_POOL_GLOBAL;
        break;
    case OH_NN_FULL_CONNECTION_HAS_BIAS:
        tensorType = OH_NN_FULL_CONNECTION_HAS_BIAS;
        break;
    case OH_NN_FULL_CONNECTION_USE_AXIS:
        tensorType = OH_NN_FULL_CONNECTION_USE_AXIS;
        break;
    case OH_NN_GELU_APPROXIMATE:
        tensorType = OH_NN_GELU_APPROXIMATE;
        break;
    case OH_NN_MAX_POOL_ROUND_MODE:
        tensorType = OH_NN_MAX_POOL_ROUND_MODE;
        break;
    case OH_NN_MAX_POOL_GLOBAL:
        tensorType = OH_NN_MAX_POOL_GLOBAL;
        break;
    case OH_NN_PAD_PADDING_MODE:
        tensorType = OH_NN_PAD_PADDING_MODE;
        break;
    case OH_NN_REDUCE_MEAN_REDUCE_TO_END:
        tensorType = OH_NN_REDUCE_MEAN_REDUCE_TO_END;
        break;
    case OH_NN_REDUCE_MEAN_COEFF:
        tensorType = OH_NN_REDUCE_MEAN_COEFF;
        break;
    case OH_NN_REDUCE_PROD_REDUCE_TO_END:
        tensorType = OH_NN_REDUCE_PROD_REDUCE_TO_END;
        break;
    case OH_NN_REDUCE_PROD_COEFF:
        tensorType = OH_NN_REDUCE_PROD_COEFF;
        break;
    case OH_NN_REDUCE_ALL_REDUCE_TO_END:
        tensorType = OH_NN_REDUCE_ALL_REDUCE_TO_END;
        break;
    case OH_NN_REDUCE_ALL_COEFF:
        tensorType = OH_NN_REDUCE_ALL_COEFF;
        break;
    case OH_NN_TOP_K_AXIS:
        tensorType = OH_NN_TOP_K_AXIS;
        break;
    case OH_NN_ARG_MAX_TOP_K:
        tensorType = OH_NN_ARG_MAX_TOP_K;
        break;
    case OH_NN_ARG_MAX_OUT_MAX_VALUE:
        tensorType = OH_NN_ARG_MAX_OUT_MAX_VALUE;
        break;
    case OH_NN_QUANT_DTYPE_CAST_AXIS:
        tensorType = OH_NN_QUANT_DTYPE_CAST_AXIS;
        break;
    case OH_NN_SLICE_AXES:
        tensorType = OH_NN_SLICE_AXES;
        break;
    case OH_NN_TILE_DIMS:
        tensorType = OH_NN_TILE_DIMS;
        break;
    case OH_NN_CROP_AXIS:
        tensorType = OH_NN_CROP_AXIS;
        break;
    case OH_NN_CROP_OFFSET:
        tensorType = OH_NN_CROP_OFFSET;
        break;
    case OH_NN_DETECTION_POST_PROCESS_INPUT_SIZE:
        tensorType = OH_NN_DETECTION_POST_PROCESS_INPUT_SIZE;
        break;
    case OH_NN_DETECTION_POST_PROCESS_SCALE:
        tensorType = OH_NN_DETECTION_POST_PROCESS_SCALE;
        break;
    case OH_NN_DETECTION_POST_PROCESS_NMS_IOU_THRESHOLD:
        tensorType = OH_NN_DETECTION_POST_PROCESS_NMS_IOU_THRESHOLD;
        break;
    case OH_NN_DETECTION_POST_PROCESS_NMS_SCORE_THRESHOLD:
        tensorType = OH_NN_DETECTION_POST_PROCESS_NMS_SCORE_THRESHOLD;
        break;
    case OH_NN_DETECTION_POST_PROCESS_MAX_DETECTIONS:
        tensorType = OH_NN_DETECTION_POST_PROCESS_MAX_DETECTIONS;
        break;
    case OH_NN_DETECTION_POST_PROCESS_DETECTIONS_PER_CLASS:
        tensorType = OH_NN_DETECTION_POST_PROCESS_DETECTIONS_PER_CLASS;
        break;
    case OH_NN_DETECTION_POST_PROCESS_MAX_CLASSES_PER_DETECTION:
        tensorType = OH_NN_DETECTION_POST_PROCESS_MAX_CLASSES_PER_DETECTION;
        break;
    case OH_NN_DETECTION_POST_PROCESS_NUM_CLASSES:
        tensorType = OH_NN_DETECTION_POST_PROCESS_NUM_CLASSES;
        break;
    case OH_NN_DETECTION_POST_PROCESS_USE_REGULAR_NMS:
        tensorType = OH_NN_DETECTION_POST_PROCESS_USE_REGULAR_NMS;
        break;
    case OH_NN_DETECTION_POST_PROCESS_OUT_QUANTIZED:
        tensorType = OH_NN_DETECTION_POST_PROCESS_OUT_QUANTIZED;
        break;
    case OH_NN_L2_NORMALIZE_AXIS:
        tensorType = OH_NN_L2_NORMALIZE_AXIS;
        break;
    case OH_NN_L2_NORMALIZE_EPSILON:
        tensorType = OH_NN_L2_NORMALIZE_EPSILON;
        break;
    case OH_NN_L2_NORMALIZE_ACTIVATION_TYPE:
        tensorType = OH_NN_L2_NORMALIZE_ACTIVATION_TYPE;
        break;
    case OH_NN_LOG_SOFTMAX_AXIS:
        tensorType = OH_NN_LOG_SOFTMAX_AXIS;
        break;
    case OH_NN_LRN_DEPTH_RADIUS:
        tensorType = OH_NN_LRN_DEPTH_RADIUS;
        break;
    case OH_NN_LRN_BIAS:
        tensorType = OH_NN_LRN_BIAS;
        break;
    case OH_NN_LRN_ALPHA:
        tensorType = OH_NN_LRN_ALPHA;
        break;
    case OH_NN_LRN_BETA:
        tensorType = OH_NN_LRN_BETA;
        break;
    case OH_NN_LRN_NORM_REGION:
        tensorType = OH_NN_LRN_NORM_REGION;
        break;
    case OH_NN_SPACE_TO_DEPTH_BLOCK_SIZE:
        tensorType = OH_NN_SPACE_TO_DEPTH_BLOCK_SIZE;
        break;
    case OH_NN_REDUCE_MAX_KEEP_DIMS:
        tensorType = OH_NN_REDUCE_MAX_KEEP_DIMS;
        break;
    case OH_NN_REDUCE_MAX_REDUCE_TO_END:
        tensorType = OH_NN_REDUCE_MAX_REDUCE_TO_END;
        break;
    case OH_NN_REDUCE_MAX_COEFF:
        tensorType = OH_NN_REDUCE_MAX_COEFF;
        break;
    case OH_NN_REDUCE_MIN_KEEP_DIMS:
        tensorType = OH_NN_REDUCE_MIN_KEEP_DIMS;
        break;
    case OH_NN_REDUCE_MIN_REDUCE_TO_END:
        tensorType = OH_NN_REDUCE_MIN_REDUCE_TO_END;
        break;
    case OH_NN_REDUCE_MIN_COEFF:
        tensorType = OH_NN_REDUCE_MIN_COEFF;
        break;
    case OH_NN_REDUCE_SUM_KEEP_DIMS:
        tensorType = OH_NN_REDUCE_SUM_KEEP_DIMS;
        break;
    case OH_NN_REDUCE_SUM_REDUCE_TO_END:
        tensorType = OH_NN_REDUCE_SUM_REDUCE_TO_END;
        break;
    case OH_NN_REDUCE_SUM_COEFF:
        tensorType = OH_NN_REDUCE_SUM_COEFF;
        break;
    case OH_NN_REDUCE_L2_KEEP_DIMS:
        tensorType = OH_NN_REDUCE_L2_KEEP_DIMS;
        break;
    case OH_NN_REDUCE_L2_REDUCE_TO_END:
        tensorType = OH_NN_REDUCE_L2_REDUCE_TO_END;
        break;
    case OH_NN_REDUCE_L2_COEFF:
        tensorType = OH_NN_REDUCE_L2_COEFF;
        break;
    default:
        napi_value napiRet;
        napi_create_int32(env, OH_NN_INVALID_PARAMETER, &napiRet);
        return napiRet;
    }

    OH_NNModel *model = OH_NNModel_Construct();

    int32_t inputDims[4] = {1, 1, 2, 3};
    NN_TensorDesc *tensorDesc = createTensorDesc(inputDims, 4, OH_NN_FLOAT32, OH_NN_FORMAT_NCHW);
    OH_NNModel_AddTensorToModel(model, tensorDesc);
    auto ret = OH_NNModel_SetTensorType(model, 0, tensorType);
    if (ret == OH_NN_SUCCESS) {
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
    }
    return nullptr;
}

napi_value OhNnQuantParamCode(napi_env env, napi_callback_info info)
{
    int32_t dimensions[3]{3, 2, 2};
    uint32_t quantCount = 1;
    uint32_t numBits = 8;
    double scale = 0.f;
    int32_t zeroPoint = 0;
    OH_NN_QuantParam quantParam = {.quantCount = quantCount, .numBits = &numBits, .scale = &scale, .zeroPoint = &zeroPoint};
    OH_NNModel *model = OH_NNModel_Construct();
    OH_NN_Tensor operand{OH_NN_FLOAT32, 3, dimensions, &quantParam, OH_NN_TENSOR};
    auto ret = OH_NNModel_AddTensor(model, &operand);
    if (ret == OH_NN_SUCCESS) {
        napi_value napiRet;
        napi_create_int32(env, ret, &napiRet);
        return napiRet;
    }
    return nullptr;
}

napi_value OhNnMemoryCode(napi_env env, napi_callback_info info)
{
    void * const data = nullptr;
    const size_t length = 256;
    OH_NNModel *model = OH_NNModel_Construct();
    OH_NNCompilation *compilation = OH_NNCompilation_Construct(model);
    OH_NNExecutor *executor = OH_NNExecutor_Construct(compilation);
    uint32_t inputIndex = 1;
    OH_NN_Memory* ohnnmemory = new OH_NN_Memory{data,length};
    try {
        OH_NNExecutor_DestroyInputMemory(executor, inputIndex, &ohnnmemory);
        napi_value napiRet; 
        napi_create_int32(env, 0, &napiRet);
        return napiRet;
    } catch (char msg) {
        return nullptr;
    }
    return nullptr;
}