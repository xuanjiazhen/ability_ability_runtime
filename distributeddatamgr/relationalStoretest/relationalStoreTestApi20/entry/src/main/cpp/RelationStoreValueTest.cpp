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

#include "napi/native_api.h"
#include <cstddef>
#include <cstdint>
#include <database/data/oh_data_value.h>
#include <database/rdb/relational_store.h>
#include <js_native_api_types.h>
#include <napi/native_api.h>
#include <vector>

const char* RDB_TEST_PATH = "/data/storage/el2/database/com.acts.actsrelationstoretest/";
char RDB_STORE_NAME[] =  "rdb_store_test.db";
char BUNDLE_NAME[] =  "com.acts.acttransactiontest";
char MODULE_NAME[] =  "";

static OH_Rdb_Config config_;

static void InitRdbConfig()
{
    config_.dataBaseDir = RDB_TEST_PATH;
    config_.storeName = RDB_STORE_NAME;
    config_.bundleName = BUNDLE_NAME;
    config_.moduleName = MODULE_NAME;
    config_.securityLevel = OH_Rdb_SecurityLevel::S1;
    config_.isEncrypt = false;
    config_.selfSize = sizeof(OH_Rdb_Config);
}

static napi_value TestEnumerate(napi_env env, napi_callback_info info)
{
    napi_value result;
    InitRdbConfig();
    OH_Rdb_Store *testRdbStore = nullptr;
    int ret = -1;
    int errCode = 0;

    testRdbStore = OH_Rdb_GetOrOpen(&config_, &errCode);

    const char* createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name UNLIMITED INT, "
                                 "score INT64)";
    
    OH_Rdb_Execute(testRdbStore, createTableSql);
    
    OH_VBucket *insertBucket = OH_Rdb_CreateValuesBucket();
    insertBucket->putInt64(insertBucket, "id", 1);
    insertBucket->putInt64(insertBucket, "score", 60);
    
    uint64_t trueForm[1] = { 18446744073709551615 };
    OH_VBucket_PutUnlimitedInt(insertBucket, "name", 0, trueForm, 1);
    
    OH_Rdb_Insert(testRdbStore, "test", insertBucket);
    
    OH_Predicates * pred = OH_Rdb_CreatePredicates("test");
    
    OH_Cursor *cursor = OH_Rdb_Query(testRdbStore, pred, NULL, 0);
    
    cursor->goToNextRow(cursor);
    
    OH_ColumnType type;
    cursor->getColumnType(cursor, 1, &type);

    if (type == OH_ColumnType::TYPE_UNLIMITED_INT) {
        ret = 0;
    }
    
    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value Destroy(napi_env env, napi_callback_info info)
{
    napi_value result;
    int ret = OH_Value_Destroy(nullptr);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value PutNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    int ret = OH_Value_PutNull(nullptr);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value PutReal(napi_env env, napi_callback_info info)
{
    napi_value result;
    double value = 1.2;
    int ret = OH_Value_PutReal(nullptr, value);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value PutText(napi_env env, napi_callback_info info)
{
    napi_value result;
    int ret = OH_Value_PutText(nullptr, "test");

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value PutBlob(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    unsigned char arr[1024] = {1, 2, 3};

    size_t length = sizeof(arr);

    int ret = OH_Value_PutBlob(value, arr, length);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value PutAsset(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    Data_Asset *asset = OH_Data_Asset_CreateOne();

    int ret = OH_Value_PutAsset(value, asset);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value PutAssets(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    Data_Asset **asset1 = OH_Data_Asset_CreateMultiple(2);
    int assetsLen = 2;
    int ret = OH_Value_PutAssets(value, asset1, assetsLen);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value PutUnlimitedInt(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    uint64_t bigInt[] = {1, 2, 3, 4, 5};
    size_t length = sizeof(bigInt);
    int ret = OH_Value_PutUnlimitedInt(value, 0, bigInt, length);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value GetType(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    OH_ColumnType type = OH_ColumnType::TYPE_ASSET;
    int ret = OH_Value_GetType(value, &type);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value IsNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    bool val = true;
    int ret = OH_Value_IsNull(value, &val);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetIntInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    int64_t val;
    int ret = OH_Value_GetInt(value, &val);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetIntTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();
    OH_Value_PutText(value, "str");

    int64_t val;
    int ret = OH_Value_GetInt(value, &val);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetIntDataTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();
    OH_Value_PutNull(value);

    int64_t val;
    int ret = OH_Value_GetInt(value, &val);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetRealInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    double val;
    int ret = OH_Value_GetReal(value, &val);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetTextInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    const char *val = nullptr;
    int ret = OH_Value_GetText(value, &val);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetTextTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    int64_t data = 100;
    OH_Value_PutInt(value, data);

    const char *val;
    int ret = OH_Value_GetText(value, &val);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetTextDataTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();
    OH_Value_PutNull(value);

    const char *val;
    int ret = OH_Value_GetText(value, &val);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetBlobInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    const uint8_t *val = nullptr;
    size_t valLength = sizeof(*val);
    int ret = OH_Value_GetBlob(value, &val, &valLength);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetBlobTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    double doubleData = 1.23;
    OH_Value_PutReal(value, doubleData);

    const uint8_t *val = nullptr;
    size_t valLength = sizeof(*val);
    int ret = OH_Value_GetBlob(value, &val, &valLength);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetBlobDataTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    const uint8_t *val = nullptr;
    size_t valLength = sizeof(*val);
    int ret = OH_Value_GetBlob(value, &val, &valLength);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetAssetInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    Data_Asset *val = nullptr;
    int ret = OH_Value_GetAsset(value, val);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetAssetTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    int64_t intData = 123;
    OH_Value_PutInt(value, intData);

    Data_Asset *val = OH_Data_Asset_CreateOne();
    int ret = OH_Value_GetAsset(value, val);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    OH_Data_Asset_DestroyOne(val);
    return result;
}

static napi_value TestGetAssetDataTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    Data_Asset *val = OH_Data_Asset_CreateOne();
    int ret = OH_Value_GetAsset(value, val);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    OH_Data_Asset_DestroyOne(val);
    return result;
}

static napi_value TestGetAssetsCountInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    size_t length;
    int ret = OH_Value_GetAssetsCount(value, &length);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetAssetsCountTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    int64_t intData = 123;
    OH_Value_PutInt(value, intData);

    size_t length;
    int ret = OH_Value_GetAssetsCount(value, &length);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetAssetsCountDataTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    size_t length;
    int ret = OH_Value_GetAssetsCount(value, &length);

    napi_create_int32(env, ret, &result);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetAssetsInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    uint32_t count = 2;
    Data_Asset **val = OH_Data_Asset_CreateMultiple(count);

    size_t inLen = 2;
    size_t outLen;
    int ret = OH_Value_GetAssets(value, val, inLen, &outLen);

    napi_create_int32(env, ret, &result);
    OH_Data_Asset_DestroyMultiple(val, count);
    return result;
}

static napi_value TestGetAssetsTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    int64_t intData = 123;
    OH_Value_PutInt(value, intData);

    uint32_t count = 2;
    Data_Asset **val = OH_Data_Asset_CreateMultiple(count);

    size_t inLen = 2;
    size_t outLen;
    int ret = OH_Value_GetAssets(value, val, inLen, &outLen);

    napi_create_int32(env, ret, &result);
    OH_Data_Asset_DestroyMultiple(val, count);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetAssetsDataTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    uint32_t count = 2;
    Data_Asset **val = OH_Data_Asset_CreateMultiple(count);

    size_t inLen = 2;
    size_t outLen;
    int ret = OH_Value_GetAssets(value, val, inLen, &outLen);

    napi_create_int32(env, ret, &result);

    OH_Data_Asset_DestroyMultiple(val, count);
    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetFloatVectorCountInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    size_t length;
    int ret = OH_Value_GetFloatVectorCount(value, &length);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetFloatVectorCountTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    int64_t intData = 123;
    OH_Value_PutInt(value, intData);

    size_t length;
    int ret = OH_Value_GetFloatVectorCount(value, &length);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetFloatVectorCountTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    size_t length;
    int ret = OH_Value_GetFloatVectorCount(value, &length);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetFloatVectorInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    float array[10];
    size_t inLen = 10;
    size_t outLen;
    int ret = OH_Value_GetFloatVector(value, array, inLen, &outLen);

    napi_create_int32(env, ret, &result);
    return result;
}

static napi_value TestGetFloatVectorTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    int64_t intData = 123;
    OH_Value_PutInt(value, intData);

    float array[10];
    size_t inLen = 10;
    size_t outLen;

    int ret = OH_Value_GetFloatVector(value, array, inLen, &outLen);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetFloatVectorTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    float array[10];
    size_t inLen = 10;
    size_t outLen;

    int ret = OH_Value_GetFloatVector(value, array, inLen, &outLen);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetUnlimitedIntBandInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;

    size_t length = 0;
    int ret = OH_Value_GetUnlimitedIntBand(value, &length);

    napi_create_int32(env, ret, &result);

    return result;
}

static napi_value TestGetUnlimitedIntBandTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    const char *val = "str";
    OH_Value_PutText(value, val);

    size_t length = 0;
    int ret = OH_Value_GetUnlimitedIntBand(value, &length);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetUnlimitedIntBandTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    size_t length = 0;
    int ret = OH_Value_GetUnlimitedIntBand(value, &length);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetUnlimitedIntInvalidParam(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = nullptr;
    
    int readDataSign = 0;
    uint64_t readData[5];
    size_t inLen = 5;
    size_t outLen;
    
    int ret = OH_Value_GetUnlimitedInt(value, &readDataSign, readData, inLen, &outLen);

    napi_create_int32(env, ret, &result);

    return result;
}

static napi_value TestGetUnlimitedIntTypeMismatch(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    const char *val = "str";
    OH_Value_PutText(value, val);

    int readDataSign = 0;
    uint64_t readData[5];
    size_t inLen = 5;
    size_t outLen;
    int ret = OH_Value_GetUnlimitedInt(value, &readDataSign, readData, inLen, &outLen);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static napi_value TestGetUnlimitedIntTypeNull(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Data_Value *value = OH_Value_Create();

    OH_Value_PutNull(value);

    int readDataSign = 0;
    uint64_t readData[5];
    size_t inLen = 5;
    size_t outLen;
    int ret = OH_Value_GetUnlimitedInt(value, &readDataSign, readData, inLen, &outLen);

    napi_create_int32(env, ret, &result);

    OH_Value_Destroy(value);
    return result;
}

static void InitDescOne(std::vector<napi_property_descriptor> &descVe)
{
    descVe.push_back({"testGetAssetsInvalidParam", nullptr, TestGetAssetsInvalidParam, nullptr, nullptr, nullptr,
                      napi_default, nullptr});
    descVe.push_back({"testGetAssetsTypeMismatch", nullptr, TestGetAssetsTypeMismatch, nullptr, nullptr, nullptr,
                      napi_default, nullptr});
    descVe.push_back({"testGetAssetsDataTypeNull", nullptr, TestGetAssetsDataTypeNull, nullptr, nullptr, nullptr,
                      napi_default, nullptr});
    descVe.push_back({"testGetFloatVectorCountInvalidParam", nullptr, TestGetFloatVectorCountInvalidParam, nullptr,
                      nullptr, nullptr, napi_default, nullptr});
    descVe.push_back({"testGetFloatVectorCountTypeMismatch", nullptr, TestGetFloatVectorCountTypeMismatch, nullptr,
                      nullptr, nullptr, napi_default, nullptr});
    descVe.push_back({"testGetFloatVectorCountTypeNull", nullptr, TestGetFloatVectorCountTypeNull, nullptr, nullptr,
                      nullptr, napi_default, nullptr});
    descVe.push_back({"testGetFloatVectorInvalidParam", nullptr, TestGetFloatVectorInvalidParam, nullptr, nullptr,
                      nullptr, napi_default, nullptr});
    descVe.push_back({"testGetFloatVectorTypeMismatch", nullptr, TestGetFloatVectorTypeMismatch, nullptr, nullptr,
                      nullptr, napi_default, nullptr});
    descVe.push_back({"testGetFloatVectorTypeNull", nullptr, TestGetFloatVectorTypeNull, nullptr, nullptr, nullptr,
                      napi_default, nullptr});
    descVe.push_back({"testGetUnlimitedIntBandInvalidParam", nullptr, TestGetUnlimitedIntBandInvalidParam, nullptr,
                      nullptr, nullptr, napi_default, nullptr});
    descVe.push_back({"testGetUnlimitedIntBandTypeMismatch", nullptr, TestGetUnlimitedIntBandTypeMismatch, nullptr,
                      nullptr, nullptr, napi_default, nullptr});
    descVe.push_back({"testGetUnlimitedIntBandTypeNull", nullptr, TestGetUnlimitedIntBandTypeNull, nullptr, nullptr,
                      nullptr, napi_default, nullptr});
    descVe.push_back({"testGetUnlimitedIntInvalidParam", nullptr, TestGetUnlimitedIntInvalidParam, nullptr, nullptr,
                      nullptr, napi_default, nullptr});
    descVe.push_back({"testGetUnlimitedIntTypeMismatch", nullptr, TestGetUnlimitedIntTypeMismatch, nullptr, nullptr,
                      nullptr, napi_default, nullptr});
    descVe.push_back({"testGetUnlimitedIntTypeNull", nullptr, TestGetUnlimitedIntTypeNull, nullptr, nullptr,
                      nullptr, napi_default, nullptr});
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> descVe = {
        {"testEnumerate", nullptr, TestEnumerate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"destroy", nullptr, Destroy, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"putNull", nullptr, PutNull, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"putReal", nullptr, PutReal, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"putText", nullptr, PutText, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"putBlob", nullptr, PutBlob, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"putAsset", nullptr, PutAsset, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"putAssets", nullptr, PutAssets, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"putUnlimitedInt", nullptr, PutUnlimitedInt, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getType", nullptr, GetType, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isNull", nullptr, IsNull, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetIntInvalidParam", nullptr, TestGetIntInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetIntTypeMismatch", nullptr, TestGetIntTypeMismatch, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetIntDataTypeNull", nullptr, TestGetIntDataTypeNull, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetRealInvalidParam", nullptr, TestGetRealInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetTextInvalidParam", nullptr, TestGetTextInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetTextTypeMismatch", nullptr, TestGetTextTypeMismatch, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetTextDataTypeNull", nullptr, TestGetTextDataTypeNull, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetBlobInvalidParam", nullptr, TestGetBlobInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetBlobTypeMismatch", nullptr, TestGetBlobTypeMismatch, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetBlobDataTypeNull", nullptr, TestGetBlobDataTypeNull, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"testGetAssetInvalidParam", nullptr, TestGetAssetInvalidParam, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"testGetAssetTypeMismatch", nullptr, TestGetAssetTypeMismatch, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"testGetAssetDataTypeNull", nullptr, TestGetAssetDataTypeNull, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"testGetAssetsCountInvalidParam", nullptr, TestGetAssetsCountInvalidParam, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"testGetAssetsCountTypeMismatch", nullptr, TestGetAssetsCountTypeMismatch, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"testGetAssetsCountDataTypeNull", nullptr, TestGetAssetsCountDataTypeNull, nullptr, nullptr, nullptr,
         napi_default, nullptr},
    };

    InitDescOne(descVe);
    napi_define_properties(env, exports, descVe.size(), descVe.data());
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "relationstoretest",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&demoModule); }
