/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "common/native_common.h"
#include "napi/native_api.h"
#include <securec.h>
#include <pthread.h>
#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <uv.h>

#include "hilog/log.h"

#define THREAD_COUNT 4
#define NUM_100 100
#define NUM_128 128
#define NUM_1024 1024
#define NUM_512 512
#define BUFFER_MAX_LEN 2097152

struct ThreadData {
    napi_env env;
    napi_value arraybuffer;
    int threadId;
    bool testPassed;
};

static pthread_mutex_t g_lock;
static napi_ref g_arraybufferRef;

static void* ThreadFunction(void* arg);
static const char TEST_STR[] = "Where there is a will, there is a way.";

static napi_value CreateBuffer(napi_env env, napi_callback_info info)
{
    const unsigned int bufferSize = sizeof(TEST_STR);
    char* copyPtr;
    napi_value napiBuffer;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&copyPtr), &napiBuffer);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create buffer");
        return nullptr;
    }
    NAPI_ASSERT(env, copyPtr, "Unable to duplicate static text for CreateBuffer.");
    int ret = memcpy_s(copyPtr, bufferSize, TEST_STR, strlen(TEST_STR) + 1);
    NAPI_ASSERT(env, ret == 0, "memcpy_s failed");
    return napiBuffer;
}

napi_value CreateBufferCopy(napi_env env, napi_callback_info info)
{
    const char* sourceData = TEST_STR;
    const unsigned int bufferSize = sizeof(TEST_STR);
    char* copyPtr;
    napi_value napiBuffer;
    napi_status status = napi_create_buffer_copy(env, bufferSize, sourceData, (void**)(&copyPtr), &napiBuffer);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create buffer");
        return nullptr;
    }
    NAPI_ASSERT(env, copyPtr, "Unable to duplicate static text for CreateBuffer.");
    int ret = memcpy_s(copyPtr, bufferSize, TEST_STR, strlen(TEST_STR) + 1);
    NAPI_ASSERT(env, ret == 0, "memcpy_s failed");
    return napiBuffer;
}

static napi_value IsBuffer(napi_env env, napi_callback_info info)
{
    napi_value args[1];
    size_t argc = 1;
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to parse arguments");
        return nullptr;
    }
    NAPI_ASSERT(env, argc == 1, "The number of arguments provided is incorrect.");
    napi_value napiBuffer = args[0];
    bool result;
    napi_status status = napi_is_buffer(env, napiBuffer, &result);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "The parameter instance is not of type buffer.");
    }
    napi_value returnValue;
    NAPI_CALL(env, napi_get_boolean(env, result, &returnValue));
    return returnValue;
}

static napi_value GetBufferInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    const unsigned int bufferSize = sizeof(TEST_STR);
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to parse arguments");
        return nullptr;
    }
    NAPI_ASSERT(env, argc == 1, "Incorrect number of parameters.");
    napi_value napiBuffer = args[0];
    char *bufferData;
    napi_value returnValue;
    size_t bufferLength;
    if (napi_get_buffer_info(env, napiBuffer, (void**)(&bufferData), &bufferLength) != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get buffer info.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_boolean(env,
                                    !strcmp(bufferData, TEST_STR) && bufferLength == bufferSize,
                                    &returnValue));
    return returnValue;
}


void FinalizeCallbackTest(napi_env env, void *data, void *hint)
{
    if (data == nullptr) {
        return;
    }
    free(data);
    data = nullptr;
}

napi_value CreateExternalBufferTest(napi_env env, napi_callback_info info)
{
    const char* sourceData = TEST_STR;
    const unsigned int bufferSize = sizeof(TEST_STR);
    char* copyPtr;
    napi_value napiBuffer;
    napi_status status = napi_create_external_buffer(env, bufferSize,
        (void*)sourceData, FinalizeCallbackTest, (void**)(&copyPtr), &napiBuffer);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create buffer");
        return nullptr;
    }
    NAPI_ASSERT(env, copyPtr, "Unable to duplicate static text for CreateExternalBufferTest.");
    int ret = memcpy_s(copyPtr, bufferSize, TEST_STR, strlen(TEST_STR) + 1);
    NAPI_ASSERT(env, ret == 0, "memcpy_s failed");
    return napiBuffer;
}

// Buffer creation edge cases and error handling functions
napi_value createBufferSizeZeroTest(napi_env env, napi_callback_info info)
{
    const unsigned int bufferSize = 0;
    char* copyPtr;
    napi_value napiBuffer;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&copyPtr), &napiBuffer);
    // 创建返回值表示状态
    napi_value retValue;
    napi_create_int32(env, status, &retValue);

    return retValue;
}

napi_value CreateBufferMaxLength(napi_env env, napi_callback_info info)
{
    const unsigned int bufferSize = BUFFER_MAX_LEN; // 最大长度
    char* copyPtr;
    napi_value napiBuffer;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&copyPtr), &napiBuffer);
    // 创建返回值表示状态
    napi_value retValue;
    napi_create_int32(env, status, &retValue);

    return retValue;
}

napi_value CreateBufferExceedMaxLength(napi_env env, napi_callback_info info)
{
    const unsigned int bufferSize = BUFFER_MAX_LEN + 1; // 超过最大长度
    char* copyPtr;
    napi_value napiBuffer;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&copyPtr), &napiBuffer);
    
    // 创建返回值表示状态
    napi_value retValue;
    napi_create_int32(env, status, &retValue);
    
    // 如果意外成功，释放内存
    if (copyPtr != nullptr) {
        free(copyPtr);
    }
    
    return retValue;
}

napi_value CreateBufferInvalidEnv(napi_env env, napi_callback_info info)
{
    const unsigned int bufferSize = 100;
    char* copyPtr;
    napi_value napiBuffer;
    napi_status status = napi_create_buffer(nullptr, bufferSize, (void**)(&copyPtr), &napiBuffer);
    
    // 创建返回值表示状态
    napi_value retValue;
    napi_create_int32(env, status, &retValue);
    
    // 如果意外成功，释放内存
    if (copyPtr != nullptr) {
        free(copyPtr);
    }
    
    return retValue;
}

napi_value CreateBufferInvalidData(napi_env env, napi_callback_info info)
{
    const unsigned int bufferSize = 100;
    napi_value napiBuffer;
    napi_status status = napi_create_buffer(env, bufferSize, nullptr, &napiBuffer);
    
    // 创建返回值表示状态
    napi_value retValue;
    napi_create_int32(env, status, &retValue);
    
    return retValue;
}

napi_value CreateBufferInvalidResult(napi_env env, napi_callback_info info)
{
    const unsigned int bufferSize = 100;
    char* copyPtr;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&copyPtr), nullptr);
    
    // 创建返回值表示状态
    napi_value retValue;
    napi_create_int32(env, status, &retValue);
    
    // 如果意外成功，释放内存
    if (copyPtr != nullptr) {
        free(copyPtr);
    }
    
    return retValue;
}

napi_value CreateInt32Test(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, NUM_100, &retValue);
    
    return retValue;
}

napi_value GetBufferInfoNonBuffer(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    const unsigned int bufferSize = sizeof(TEST_STR);
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to parse arguments");
        return nullptr;
    }
    NAPI_ASSERT(env, argc == 1, "Incorrect number of parameters.");
    napi_value napiBuffer = args[0];
    char *bufferData;
    napi_value returnValue;
    size_t bufferLength;
    napi_status status = napi_get_buffer_info(env, napiBuffer, (void**)(&bufferData), &bufferLength);
    if (status == napi_ok) {
        napi_throw_error(env, nullptr, "Int cannot get buffer info.");
        return nullptr;
    }

    napi_create_int32(env, status, &returnValue);
    return returnValue;
}

napi_value TestMultipleOperations(napi_env env, napi_callback_info info)
{
    // 创建初始buffer
    size_t initialLength = 256;
    char* initialData;

    napi_value reOk;
    napi_value reFalse;

    NAPI_CALL(env, napi_create_int32(env, napi_ok, &reOk));
    NAPI_CALL(env, napi_create_int32(env, 1, &reFalse));

    napi_value initialBuffer;
    napi_status status = napi_create_buffer(env, initialLength, (void**)(&initialData), &initialBuffer);
    if (status != napi_ok) {
        return reFalse;
    }

    // 修改buffer内容
    memset_s(initialData, initialLength, 0xAA, initialLength);

    // 创建拷贝
    napi_value copiedBuffer;
    char* copiedData;
    status = napi_create_buffer_copy(env, initialLength,
        (void*)(initialData), (void**)(&copiedData), &copiedBuffer);
    if (status != napi_ok) {
        return reFalse;
    }

    // 验证拷贝内容
    char* retrievedData;
    size_t retrievedLength;
    status = napi_get_buffer_info(env, copiedBuffer, (void**)(&retrievedData), &retrievedLength);
    if (status != napi_ok) {
        return reFalse;
    }
    if (memcmp(initialData, retrievedData, initialLength) != 0) {
        return reFalse;
    }

    // 修改原始buffer不应影响拷贝
    memset_s(initialData, initialLength, 0xBB, initialLength);
    if (memcmp(initialData, retrievedData, initialLength) != 0) {
        return reOk;
    }
    return reFalse;
}

napi_value TestExternalArrayBuffer(napi_env env, napi_callback_info info)
{
    size_t extArrLength = NUM_128;
    void* extArrData = malloc(extArrLength);
    memset_s(extArrData, extArrLength, 0xCC, extArrLength);

    bool extFinalizeCalled = false;
    napi_finalize extFinalizeCb = [](napi_env env, void* data, void* hint) {
        free(data);
        *((bool*)hint) = true;
    };

    napi_value extArrBuffer;
    napi_status status;
    napi_value reOk;
    napi_value reFalse;

    NAPI_CALL(env, napi_create_int32(env, napi_ok, &reOk));
    NAPI_CALL(env, napi_create_int32(env, 1, &reFalse));

    status = napi_create_external_arraybuffer(env, extArrData, extArrLength,
        extFinalizeCb, &extFinalizeCalled, &extArrBuffer);
    if (status != napi_ok) {
        return reFalse;
    }

    // 验证buffer内容
    void* retrievedExtData;
    size_t retrievedExtLength;
    status = napi_get_buffer_info(env, extArrBuffer, &retrievedExtData, &retrievedExtLength);
    if (status != napi_ok) {
        return reFalse;
    }
    if (memcmp(retrievedExtData, extArrData, retrievedExtLength) != 0) {
        return reFalse;
    }
    if (retrievedExtLength != extArrLength) {
        return reFalse;
    }

    // 强制GC并验证finalize回调
    // ... 执行GC操作 ...
    // 获取全局对象
    napi_value global;
    napi_get_global(env, &global);
    napi_value gc_func;
    napi_get_named_property(env, global, "gc", &gc_func);
    napi_call_function(env, global, gc_func, 0, nullptr, nullptr);
    if (extFinalizeCalled != true) {
        return reFalse;
    }
    return reOk;
}

// 多线程环境下的ArrayBuffer线程安全性测试
#define NUM_1 1
#define NUM_2 2
#define NUM_3 3
#define NUM_4 4
#define THREAD_COUNT 5
#define BUFFER_SIZE 1024
#define TEST_DATA "This is a test string for ArrayBuffer operations"

// 线程共享数据
typedef struct {
    napi_env env;
    napi_value buffer;
    bool testCompleted;
} ThreadSharedData;

// 线程函数1: 创建ArrayBuffer
void* ThreadCreateBuffer(void* arg)
{
    ThreadSharedData* sharedData  = (ThreadSharedData*)arg;
    napi_env env = sharedData ->env;

    void* data = nullptr;
    napi_value buffer;
    napi_status status = napi_create_buffer(env, BUFFER_SIZE, &data, &buffer);
    if (status != napi_ok) {
        // 错误处理
        OH_LOG_INFO(LOG_APP, "testTag napi_create_buffer failed.");
        return nullptr;
    }

    // 填充测试数据
    int ret = memcpy_s(data, strlen(TEST_DATA), TEST_DATA, strlen(TEST_DATA));
    NAPI_ASSERT(env, ret == 0, "memcpy_s failed");

    // 存储创建的buffer
    sharedData->buffer = buffer;
    return nullptr;
}

// 线程函数2: 创建ArrayBuffer拷贝
void* ThreadCreateBufferCopy(void* arg)
{
    ThreadSharedData* sharedData  = (ThreadSharedData*)arg;
    napi_env env = sharedData ->env;
    
    void* resultData = nullptr;
    napi_value buffer;
    napi_status status = napi_create_buffer_copy(env, strlen(TEST_DATA) + 1,
        TEST_DATA, &resultData, &buffer);
    if (status != napi_ok) {
        // 错误处理
        OH_LOG_INFO(LOG_APP, "testTag napi_create_buffer_copy failed.");
        return nullptr;
    }
    
    // 验证拷贝的数据
    if (strcmp((const char*)resultData, TEST_DATA) != 0) {
        OH_LOG_INFO(LOG_APP, "testTag 数据不一致.");
        // 数据不一致
        return nullptr;
    }
    
    return nullptr;
}

// 线程函数3: 创建外部ArrayBuffer
void* ThreadCreateExternalBuffer(void* arg)
{
    ThreadSharedData* sharedData  = (ThreadSharedData*)arg;
    napi_env env = sharedData ->env;
    
    char* externalData = static_cast<char*>(malloc(strlen(TEST_DATA) + 1));
    strcpy_s(externalData, strlen(TEST_DATA) + 1, TEST_DATA);
    
    napi_value buffer;
    napi_finalize extFinalizeCb = [](napi_env env, void* data, void* hint) {
        free(data);
    };
    napi_status status = napi_create_external_buffer(env, strlen(TEST_DATA) + 1,
        externalData, extFinalizeCb, nullptr, &buffer);
    if (status != napi_ok) {
        free(externalData);
        return nullptr;
    }
    
    return nullptr;
}

// 线程函数4: 获取ArrayBuffer信息
void* ThreadGetBufferInfo(void* arg)
{
    ThreadSharedData* sharedData  = (ThreadSharedData*)arg;
    napi_env env = sharedData ->env;
    
    if (sharedData ->buffer == nullptr) {
        return nullptr;
    }
    
    void* data = nullptr;
    size_t length = 0;
    napi_status status = napi_get_buffer_info(env, sharedData ->buffer, &data, &length);
    if (status != napi_ok) {
        OH_LOG_INFO(LOG_APP, "testTag napi_get_buffer_info failed.");
        return nullptr;
    }
    
    // 验证数据
    if (length != BUFFER_SIZE || data == nullptr) {
        OH_LOG_INFO(LOG_APP, "testTag 数据不一致.");
        return nullptr;
    }
    
    return nullptr;
}

// 线程函数5: 检查是否为ArrayBuffer
void* ThreadIsBuffer(void* arg)
{
    ThreadSharedData* sharedData  = (ThreadSharedData*)arg;
    napi_env env = sharedData ->env;
    
    bool isBuffer = false;
    napi_status status = napi_is_buffer(env, sharedData ->buffer, &isBuffer);
    if (status != napi_ok || !isBuffer) {
        OH_LOG_INFO(LOG_APP, "testTag napi_is_buffer failed.");
        return nullptr;
    }
    
    // 测试完成标志
    sharedData ->testCompleted = true;
    
    return nullptr;
}

// 主测试函数
napi_value TestArrayBufferThreadSafety(napi_env env, napi_callback_info info)
{
    // 初始化共享数据
    ThreadSharedData sharedData  = {env, nullptr, false};
    
    // 创建线程
    pthread_t threads[THREAD_COUNT];
    
    // 线程1: 创建ArrayBuffer
    pthread_create(&threads[0], nullptr, ThreadCreateBuffer, &sharedData);
    
    // 等待线程1完成
    pthread_join(threads[0], nullptr);
    
    // 创建其他线程
    pthread_create(&threads[NUM_1], nullptr, ThreadCreateBufferCopy, &sharedData);
    pthread_create(&threads[NUM_2], nullptr, ThreadCreateExternalBuffer, &sharedData);
    pthread_create(&threads[NUM_3], nullptr, ThreadGetBufferInfo, &sharedData);
    pthread_create(&threads[NUM_4], nullptr, ThreadIsBuffer, &sharedData);
    
    // 等待所有线程完成
    for (int i = NUM_1; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    // 返回测试结果
    napi_value result;
    napi_get_boolean(env, sharedData .testCompleted, &result);
    return result;
}

// ArrayBuffer和外部ArrayBuffer比对测试
void FinalizeCallbackComparison(napi_env env, void* finalizeData, void* finalizeHint)
{
    free(finalizeData);
}

static napi_status g_status;
static napi_value g_reOk;
static napi_value g_reFalse;
static void* g_bufferData = nullptr;
static size_t g_bufferSize = NUM_1024;
static napi_value g_normalArraybuffer;
static size_t g_externalSize = NUM_512;
static void* g_externalData = malloc(g_externalSize);
static napi_value g_externalArraybuffer;

napi_value testDetach(napi_env env, napi_callback_info info)
{
    // 4. 获取 ArrayBuffer 信息
    void* retrievedData = nullptr;
    size_t retrievedLength = 0;
    
    g_status = napi_get_arraybuffer_info(env, g_normalArraybuffer, &retrievedData, &retrievedLength);
    if (g_status != napi_ok || retrievedData != g_bufferData || retrievedLength != g_bufferSize) {
        OH_LOG_INFO(LOG_APP, "napi_get_arraybuffer_info(g_normalArraybuffer) != ok.");
        return g_reFalse;
    }
    
    g_status = napi_get_arraybuffer_info(env, g_externalArraybuffer, &retrievedData, &retrievedLength);
    if (g_status != napi_ok || retrievedData != g_externalData || retrievedLength != g_externalSize) {
        OH_LOG_INFO(LOG_APP, "napi_get_arraybuffer_info(g_normalArraybuffer) != ok.");
        return g_reFalse;
    }

    // 5. 测试 ArrayBuffer 分离
    napi_detach_arraybuffer(env, g_normalArraybuffer);
    
    bool isDetached = false;
    g_status = napi_is_detached_arraybuffer(env, g_normalArraybuffer, &isDetached);
    if (g_status != napi_ok || !isDetached) {
        OH_LOG_INFO(LOG_APP, "napi_is_detached_arraybuffer(g_normalArraybuffer) != ok.");
        return g_reFalse;
    }
    
    // 尝试分离外部 ArrayBuffer (应该失败)
    g_status = napi_detach_arraybuffer(env, g_externalArraybuffer);
    if (g_status == napi_ok) {
        OH_LOG_INFO(LOG_APP, "napi_detach_arraybuffer(g_externalArraybuffer) == ok.");
        return g_reFalse;
    }
    
    // 验证外部 ArrayBuffer 未分离
    g_status = napi_is_detached_arraybuffer(env, g_externalArraybuffer, &isDetached);
    if (g_status != napi_ok || isDetached) {
        OH_LOG_INFO(LOG_APP, "napi_is_detached_arraybuffer(g_externalArraybuffer) != ok.");
        return g_reFalse;
    }
    return g_reOk;
}

napi_value TestArrayBufferComparison(napi_env env, napi_callback_info info)
{
    NAPI_CALL(env, napi_create_int32(env, napi_ok, &g_reOk));
    NAPI_CALL(env, napi_create_int32(env, 1, &g_reFalse));
    
    // 1. 创建普通 ArrayBuffer
    napi_create_arraybuffer(env, g_bufferSize, &g_bufferData, &g_normalArraybuffer);
    memset_s(g_bufferData, g_bufferSize, 0xAA, g_bufferSize); // 填充测试数据

    // 2. 创建外部 ArrayBuffer
    memset_s(g_externalData, g_externalSize, 0xBB, g_externalSize);
    g_status = napi_create_external_arraybuffer(env, g_externalData, g_externalSize,
        FinalizeCallbackComparison, nullptr, &g_externalArraybuffer);

    // 3. 验证 ArrayBuffer 类型
    bool isAb = false;
    g_status = napi_is_arraybuffer(env, g_normalArraybuffer, &isAb);
    if (g_status != napi_ok || !isAb) {
        OH_LOG_INFO(LOG_APP, "napi_is_arraybuffer(g_normalArraybuffer) != ok.");
        return g_reFalse;
    }
    
    // 测试外部 ArrayBuffer
    g_status = napi_is_arraybuffer(env, g_externalArraybuffer, &isAb);
    if (g_status != napi_ok || !isAb) {
        OH_LOG_INFO(LOG_APP, "napi_is_arraybuffer(g_externalArraybuffer) != ok.");
        return g_reFalse;
    }

    // 6. 返回测试结果
    return testDetach(env, info);
}

napi_value NapiArrayBufferTest01(napi_env env, napi_callback_info info)
{
    napi_value arraybuffer;
    void* data = nullptr;
    const size_t byteLength = 1024;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_arraybuffer failed");
    
    bool isArraybuffer = false;
    status = napi_is_arraybuffer(env, arraybuffer, &isArraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_is_arraybuffer failed");
    NAPI_ASSERT(env, isArraybuffer, "Created object is not ArrayBuffer");
    
    void* retrievedData = nullptr;
    size_t retrievedLength = 0;
    status = napi_get_arraybuffer_info(env, arraybuffer, &retrievedData, &retrievedLength);
    NAPI_ASSERT(env, status == napi_ok, "napi_get_arraybuffer_info failed");

    NAPI_ASSERT(env, data == retrievedData, "Data pointer mismatch");
    NAPI_ASSERT(env, byteLength == retrievedLength, "Buffer length mismatch");

    bool isDetached = true;
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "napi_is_detached_arraybuffer failed");
    NAPI_ASSERT(env, !isDetached, "ArrayBuffer should not be detached");

    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value NapiArrayBufferTest02(napi_env env, napi_callback_info info)
{
    napi_value arraybuffer;
    void* data = nullptr;
    const size_t byteLength = 512;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_arraybuffer failed");
    
    const char* testData = "Hello, ArrayBuffer!";
    size_t dataLen = strlen(testData) + 1;
    if (dataLen <= byteLength) {
        errno_t memcpyResult = memcpy_s(data, byteLength, testData, dataLen);
        NAPI_ASSERT(env, memcpyResult == 0, "memcpy_s failed");
    }
    
    bool preDetached = true;
    status = napi_is_detached_arraybuffer(env, arraybuffer, &preDetached);
    NAPI_ASSERT(env, status == napi_ok, "Pre-detach check failed");
    NAPI_ASSERT(env, !preDetached, "ArrayBuffer should not be detached initially");
    
    status = napi_detach_arraybuffer(env, arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_detach_arraybuffer failed");
    
    bool isDetached = false;
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "Post-detach check failed");
    NAPI_ASSERT(env, isDetached, "ArrayBuffer should be detached");
    
    void* retrievedData = nullptr;
    size_t retrievedLength = 1;
    status = napi_get_arraybuffer_info(env, arraybuffer, &retrievedData, &retrievedLength);
    NAPI_ASSERT(env, status == napi_ok, "napi_get_arraybuffer_info failed after detach");
    
    bool success = true;
    if (retrievedData != nullptr) {
        success = false;
    }
    if (retrievedLength != 0) {
        success = false;
    }
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

napi_value NapiArrayBufferTest03(napi_env env, napi_callback_info info)
{
    napi_value arraybuffer;
    void* data = nullptr;
    const size_t byteLength = 1024;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_arraybuffer failed");
    
    const char* testData = "Test data for ArrayBuffer";
    size_t dataLen = strlen(testData) + 1;
    if (dataLen <= byteLength) {
        errno_t memcpyResult = memcpy_s(data, byteLength, testData, dataLen);
        NAPI_ASSERT(env, memcpyResult == 0, "memcpy_s failed");
    }
    
    bool dataOk = (strcmp(static_cast<const char*>(data), testData) == 0);
    NAPI_ASSERT(env, dataOk, "Data verification failed");
    
    bool isArraybuffer = false;
    status = napi_is_arraybuffer(env, arraybuffer, &isArraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_is_arraybuffer failed");
    NAPI_ASSERT(env, isArraybuffer, "Object should be ArrayBuffer");
    
    void* retrievedData = nullptr;
    size_t retrievedLength = 0;
    status = napi_get_arraybuffer_info(env, arraybuffer, &retrievedData, &retrievedLength);
    NAPI_ASSERT(env, status == napi_ok, "napi_get_arraybuffer_info failed");
    NAPI_ASSERT(env, retrievedData == data, "Data pointer should match");
    NAPI_ASSERT(env, retrievedLength == byteLength, "Length should match");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value NapiArrayBufferTest04(napi_env env, napi_callback_info info)
{
    napi_value arraybuffer;
    void* data = nullptr;
    const size_t byteLength = 256;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_arraybuffer failed");
    
    status = napi_detach_arraybuffer(env, arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "First detach should succeed");
    
    bool isDetached = false;
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "napi_is_detached_arraybuffer failed after first detach");
    NAPI_ASSERT(env, isDetached, "ArrayBuffer should be detached after first detach");
    
    void* retrievedData = nullptr;
    size_t retrievedLength = 1;
    status = napi_get_arraybuffer_info(env, arraybuffer, &retrievedData, &retrievedLength);
    NAPI_ASSERT(env, status == napi_ok, "napi_get_arraybuffer_info failed after first detach");
    NAPI_ASSERT(env, retrievedData == nullptr, "Data pointer should be null after detach");
    NAPI_ASSERT(env, retrievedLength == 0, "Length should be 0 after detach");
    
    status = napi_detach_arraybuffer(env, arraybuffer);
    bool secondDetachFailed = (status == napi_invalid_arg);
    
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "napi_is_detached_arraybuffer failed after second detach attempt");
    NAPI_ASSERT(env, isDetached, "ArrayBuffer should remain detached");
    
    status = napi_detach_arraybuffer(env, arraybuffer);
    bool thirdDetachFailed = (status == napi_invalid_arg);
    
    napi_value result;
    napi_get_boolean(env, secondDetachFailed && thirdDetachFailed, &result);
    return result;
}

napi_value NapiArrayBufferTest05(napi_env env, napi_callback_info info)
{
    napi_value arraybuffer;
    void* data = nullptr;
    const size_t byteLength = 0;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_arraybuffer failed for zero-length buffer");
    
    bool isArraybuffer = false;
    status = napi_is_arraybuffer(env, arraybuffer, &isArraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_is_arraybuffer failed");
    NAPI_ASSERT(env, isArraybuffer, "Zero-length buffer should be ArrayBuffer");
    
    void* retrievedData = nullptr;
    size_t retrievedLength = 1;
    status = napi_get_arraybuffer_info(env, arraybuffer, &retrievedData, &retrievedLength);
    NAPI_ASSERT(env, status == napi_ok, "napi_get_arraybuffer_info failed");
    
    bool lengthOk = (retrievedLength == 0);
    NAPI_ASSERT(env, lengthOk, "Zero-length buffer should have length 0");

    bool pointerOk = true;
    bool isDetached = true;
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "napi_is_detached_arraybuffer failed");
    NAPI_ASSERT(env, !isDetached, "Zero-length buffer should not be detached");
    
    status = napi_detach_arraybuffer(env, arraybuffer);
    bool detachOk = (status == napi_ok);
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "Post-detach check failed");
    NAPI_ASSERT(env, isDetached, "Should be detached after detach");
    
    void* detachedData;
    size_t detachedLength;
    status = napi_get_arraybuffer_info(env, arraybuffer, &detachedData, &detachedLength);
    NAPI_ASSERT(env, status == napi_ok, "napi_get_arraybuffer_info after detach failed");
    NAPI_ASSERT(env, detachedData == nullptr, "Data should be null after detach");
    NAPI_ASSERT(env, detachedLength == 0, "Length should be 0 after detach");
    
    napi_value result;
    napi_get_boolean(env, lengthOk && pointerOk, &result);
    return result;
}

napi_value NapiArrayBufferTest06(napi_env env, napi_callback_info info)
{
    napi_value arraybuffer;
    void* data = nullptr;
    const size_t byteLength = 128;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "First napi_create_arraybuffer failed");

    const char* testData = "First buffer";
    size_t dataLen = strlen(testData) + 1;
    if (dataLen <= byteLength) {
        errno_t memcpyResult = memcpy_s(data, byteLength, testData, dataLen);
        NAPI_ASSERT(env, memcpyResult == 0, "memcpy_s failed");
    }

    status = napi_detach_arraybuffer(env, arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "First detach failed");

    bool isDetached = false;
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "First detach status check failed");
    NAPI_ASSERT(env, isDetached, "First buffer should be detached");

    void* secondData = nullptr;
    size_t secondByteLength = 64;

    status = napi_create_arraybuffer(env, secondByteLength, &secondData, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "Second napi_create_arraybuffer failed");

    const char* testData2 = "Second buffer";
    size_t dataLen2 = strlen(testData2) + 1;
    if (dataLen2 <= secondByteLength) {
        errno_t memcpyResult = memcpy_s(secondData, secondByteLength, testData2, dataLen2);
        NAPI_ASSERT(env, memcpyResult == 0, "memcpy_s failed");
    }

    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "Second buffer status check failed");
    NAPI_ASSERT(env, !isDetached, "Second buffer should not be detached");
    
    void* retrievedData = nullptr;
    size_t retrievedLength = 0;
    status = napi_get_arraybuffer_info(env, arraybuffer, &retrievedData, &retrievedLength);
    NAPI_ASSERT(env, status == napi_ok, "napi_get_arraybuffer_info for second buffer failed");

    bool success = true;
    if (retrievedLength != secondByteLength) {
        success = false;
    }

    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

napi_value NapiArrayBufferTest07(napi_env env, napi_callback_info info)
{
    pthread_mutex_init(&g_lock, nullptr);
    
    void* bufferData;
    napi_value arraybuffer;
    napi_status status = napi_create_arraybuffer(env, BUFFER_SIZE, &bufferData, &arraybuffer);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_arraybuffer failed");
    
    memset_s(bufferData, BUFFER_SIZE, 0, BUFFER_SIZE);
    
    status = napi_create_reference(env, arraybuffer, 1, &g_arraybufferRef);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_reference failed");
    
    pthread_t threads[THREAD_COUNT];
    ThreadData threadData[THREAD_COUNT];
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        threadData[i].env = env;
        threadData[i].arraybuffer = arraybuffer;
        threadData[i].threadId = i;
        threadData[i].testPassed = true;
        
        int err = pthread_create(&threads[i], nullptr, ThreadFunction, &threadData[i]);
        if (err != 0) {
            napi_throw_error(env, nullptr, "Failed to create thread");
            break;
        }
    }
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    bool isDetached;
    status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    NAPI_ASSERT(env, status == napi_ok, "Final status check failed");
    
    napi_delete_reference(env, g_arraybufferRef);
    pthread_mutex_destroy(&g_lock);
    
    bool allPassed = true;
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (!threadData[i].testPassed) {
            allPassed = false;
            break;
        }
    }

    napi_value result;
    napi_get_boolean(env, allPassed, &result);
    return result;
}

static void* ThreadFunction(void* arg)
{
    ThreadData* threadData = static_cast<ThreadData*>(arg);
    napi_env env = threadData->env;
    napi_value arraybuffer = threadData->arraybuffer;
    
    pthread_mutex_lock(&g_lock);
    
    bool isDetached = false;
    napi_status status = napi_is_detached_arraybuffer(env, arraybuffer, &isDetached);
    if (status != napi_ok || isDetached) {
        threadData->testPassed = false;
    }
    
    pthread_mutex_unlock(&g_lock);
    
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("CreateBuffer", CreateBuffer),
        DECLARE_NAPI_FUNCTION("CreateBufferCopy", CreateBufferCopy),
        DECLARE_NAPI_FUNCTION("IsBuffer", IsBuffer),
        DECLARE_NAPI_FUNCTION("GetBufferInfo", GetBufferInfo),
        DECLARE_NAPI_FUNCTION("CreateExternalBufferTest", CreateExternalBufferTest),
        DECLARE_NAPI_FUNCTION("createBufferSizeZeroTest", createBufferSizeZeroTest),
        DECLARE_NAPI_FUNCTION("CreateBufferMaxLength", CreateBufferMaxLength),
        DECLARE_NAPI_FUNCTION("CreateBufferExceedMaxLength", CreateBufferExceedMaxLength),
        DECLARE_NAPI_FUNCTION("CreateBufferInvalidEnv", CreateBufferInvalidEnv),
        DECLARE_NAPI_FUNCTION("CreateBufferInvalidData", CreateBufferInvalidData),
        DECLARE_NAPI_FUNCTION("CreateBufferInvalidResult", CreateBufferInvalidResult),
        DECLARE_NAPI_FUNCTION("CreateInt32Test", CreateInt32Test),
        DECLARE_NAPI_FUNCTION("GetBufferInfoNonBuffer", GetBufferInfoNonBuffer),
        DECLARE_NAPI_FUNCTION("TestMultipleOperations", TestMultipleOperations),
        DECLARE_NAPI_FUNCTION("TestExternalArrayBuffer", TestExternalArrayBuffer),
        DECLARE_NAPI_FUNCTION("TestArrayBufferThreadSafety", TestArrayBufferThreadSafety),
        DECLARE_NAPI_FUNCTION("TestArrayBufferComparison", TestArrayBufferComparison),
        DECLARE_NAPI_FUNCTION("NapiArrayBufferTest01", NapiArrayBufferTest01),
        DECLARE_NAPI_FUNCTION("NapiArrayBufferTest02", NapiArrayBufferTest02),
        DECLARE_NAPI_FUNCTION("NapiArrayBufferTest03", NapiArrayBufferTest03),
        DECLARE_NAPI_FUNCTION("NapiArrayBufferTest04", NapiArrayBufferTest04),
        DECLARE_NAPI_FUNCTION("NapiArrayBufferTest05", NapiArrayBufferTest05),
        DECLARE_NAPI_FUNCTION("NapiArrayBufferTest06", NapiArrayBufferTest06),
        DECLARE_NAPI_FUNCTION("NapiArrayBufferTest07", NapiArrayBufferTest07),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(*properties), properties));
    return exports;
}
EXTERN_C_END

static napi_module
demoModule = {
.nm_version = 1,
.nm_flags = 0,
.nm_filename = nullptr,
.nm_register_func = Init,
.nm_modname = "napitestex",
.nm_priv = ((void *)0),
.reserved = {
0 },
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
napi_module_register(& demoModule);
}
