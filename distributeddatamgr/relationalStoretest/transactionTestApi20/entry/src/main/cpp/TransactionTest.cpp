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

#include "TransactionTest.h"
#include "napi/native_api.h"
#include <cstdint>
#include <database/data/oh_data_values.h>
#include <database/rdb/oh_rdb_transaction.h>
#include <database/rdb/oh_rdb_types.h>
#include <database/rdb/relational_store.h>
#include <database/rdb/relational_store_error_code.h>
#include <hilog/log.h>
#include <js_native_api_types.h>
#include <napi/native_api.h>

int g_tSInvalidParamCode = 14800001;
int g_tSAlreadyClosedCode = 14800050;
int g_tSErrorCode = 14800000;

namespace TransactionTest {
const char *g_rdbTest = "/data/storage/el2/database/com.acts.acttransactiontest/";
char g_rdbStoreName[] = "rdb_store_test.db";
char g_bundleName[] = "com.acts.acttransactiontest";
char g_moduleName[] = "";
OH_Rdb_Config g_config_;
} // namespace TransactionTest

void AddTransactionTest1(std::vector<napi_property_descriptor> &descData) {
    descData.push_back({"testDestroyOptionsInvalidParam", nullptr, TestDestroyOptionsInvalidParam, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back({"testDestroyOptionsSuccess", nullptr, TestDestroyOptionsSuccess, nullptr, nullptr, nullptr,
                        napi_default, nullptr});
    descData.push_back({"testTransCommitInvalidParam", nullptr, TestTransCommitInvalidParam, nullptr, nullptr, nullptr,
                        napi_default, nullptr});
    descData.push_back({"testCommitTransDatabaseClosed", nullptr, TestCommitTransDatabaseClosed, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back({"testTransRollbackInvalidParam", nullptr, TestTransRollbackInvalidParam, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back({"testTransRollbackDatabaseClosed", nullptr, TestTransRollbackDatabaseClosed, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back(
        {"testInsertInvalidParam", nullptr, TestInsertInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testInsertDatabaseClosed", nullptr, TestInsertDatabaseClosed, nullptr, nullptr, nullptr,
                        napi_default, nullptr});
    descData.push_back(
        {"testTransInsertError", nullptr, TestTransInsertError, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testTransInsertWithConflictResolutionSuccess", nullptr,
                        TestTransInsertWithConflictResolutionSuccess, nullptr, nullptr, nullptr, napi_default,
                        nullptr});
    descData.push_back({"testInsertWithConflictResolutionError", nullptr, TestInsertWithConflictResolutionError,
                        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testInsertWithConflictResolutionDatabaseClosed", nullptr,
                        TestInsertWithConflictResolutionDatabaseClosed, nullptr, nullptr, nullptr, napi_default,
                        nullptr});
    descData.push_back({"testTransBatchInsertInvalidParam", nullptr, TestTransBatchInsertInvalidParam, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back({"testTransBatchInsertError", nullptr, TestTransBatchInsertError, nullptr, nullptr, nullptr,
                        napi_default, nullptr});
    descData.push_back({"testTransBatchInsertDatabaseClosed", nullptr, TestTransBatchInsertDatabaseClosed, nullptr,
                        nullptr, nullptr, napi_default, nullptr});
}

void AddTransactionTest2(std::vector<napi_property_descriptor> &descData) {
    descData.push_back(
        {"testUpdateInvalidParam", nullptr, TestUpdateInvalidParam, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back(
        {"testTransUpdateError", nullptr, TestTransUpdateError, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testTransUpdateDatabaseClosed", nullptr, TestTransUpdateDatabaseClosed, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back({"testUpdateWithConflictResolutionInvalidParam", nullptr,
                        TestUpdateWithConflictResolutionInvalidParam, nullptr, nullptr, nullptr, napi_default,
                        nullptr});
    descData.push_back({"testTransUpdateWithConflictResolutionError", nullptr,
                        TestTransUpdateWithConflictResolutionError, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testTransUpdateWithConflictResolutionDatabaseClosed", nullptr,
                        TestTransUpdateWithConflictResolutionDatabaseClosed, nullptr, nullptr, nullptr, napi_default,
                        nullptr});
    descData.push_back({"testTransDeleteInvalidParam", nullptr, TestTransDeleteInvalidParam, nullptr, nullptr, nullptr,
                        napi_default, nullptr});
    descData.push_back(
        {"testTransDeleteError", nullptr, TestTransDeleteError, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testTransDeleteDatabaseClosed", nullptr, TestTransDeleteDatabaseClosed, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back({"testTransExecuteInvalidParam", nullptr, TestTransExecuteInvalidParam, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back(
        {"testTransExecuteError", nullptr, TestTransExecuteError, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testTransExecuteDatabaseClosed", nullptr, TestTransExecuteDatabaseClosed, nullptr, nullptr,
                        nullptr, napi_default, nullptr});
    descData.push_back({"testDestroyInvalidParam", nullptr, TestDestroyInvalidParam, nullptr, nullptr, nullptr,
                        napi_default, nullptr});
}

void AddTransactionTest(std::vector<napi_property_descriptor> &descData) {
    AddTransactionTest1(descData);
    AddTransactionTest2(descData);
}

napi_value TestDestroyOptionsInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_RDB_TransOptions *options = nullptr;

    int ret = OH_RdbTrans_DestroyOptions(options);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);
    return result;
}

napi_value TestDestroyOptionsSuccess(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    int ret = OH_RdbTrans_DestroyOptions(options);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_OK ? 0 : -1, &result);

    return result;
}

napi_value TestTransCommitInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    int ret = OH_RdbTrans_Commit(trans);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);

    return result;
}

napi_value TestCommitTransDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    char table[] = "test";
    int64_t rowId = -1;

    OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);

    OH_Rdb_CloseStore(testRdbStore);

    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    int ret = OH_RdbTrans_Commit(trans);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestTransRollbackInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    int ret = OH_RdbTrans_Rollback(trans);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);

    return result;
}

napi_value TestTransRollbackDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 FLOAT, "
                                 "data3 INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    double data2Value = 99.9;
    valueBucket->putReal(valueBucket, "data2", data2Value);
    int64_t data3Value = 100;
    valueBucket->putInt64(valueBucket, "data3", data3Value);

    char table[] = "test";
    int64_t rowId = -1;
    int res = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Insert:%{public}d", res);

    int ccc = OH_RdbTrans_Commit(trans);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Commit:%{public}d", ccc);

    int ret = OH_RdbTrans_Rollback(trans);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Rollback:%{public}d", ret);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestInsertInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    OH_VBucket *ValueBucket = OH_Rdb_CreateValuesBucket();

    const char *table = "test";

    int64_t rowId = -1;

    int ret = OH_RdbTrans_Insert(trans, table, ValueBucket, &rowId);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);

    ValueBucket->destroy(ValueBucket);
    return result;
}

napi_value TestInsertDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    OH_Rdb_CloseStore(testRdbStore);

    char table[] = "test";
    int64_t rowId = -1;
    int ret = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);

    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestTransInsertError(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    char table[] = "test1";
    int64_t rowId = -1;
    int ret = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ERROR ? g_tSErrorCode : -1, &result);
    return result;
}

napi_value TestTransInsertWithConflictResolutionSuccess(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    char table[] = "test";
    int64_t rowId = -1;

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int ret = OH_RdbTrans_InsertWithConflictResolution(trans, table, valueBucket, resolution, &rowId);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_InsertWithConflictResolution:%{public}d",
                 ret);
    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_OK ? 0 : -1, &result);
    return result;
}

napi_value TestInsertWithConflictResolutionError(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    char table[] = "test1";
    int64_t rowId = -1;

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int ret = OH_RdbTrans_InsertWithConflictResolution(trans, table, valueBucket, resolution, &rowId);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ERROR ? g_tSErrorCode : -1, &result);
    return result;
}

napi_value TestInsertWithConflictResolutionDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    OH_Rdb_CloseStore(testRdbStore);

    char table[] = "test";
    int64_t rowId = -1;

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int ret = OH_RdbTrans_InsertWithConflictResolution(trans, table, valueBucket, resolution, &rowId);

    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestTransBatchInsertInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    const char *table = "test";

    OH_Data_VBuckets *rows = OH_VBuckets_Create();

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int64_t rowId = -1;

    int ret = OH_RdbTrans_BatchInsert(trans, table, rows, resolution, &rowId);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);

    OH_VBuckets_Destroy(rows);
    return result;
}

napi_value TestTransBatchInsertError(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    OH_Data_VBuckets *valueBuckets = OH_VBuckets_Create();

    int pt = OH_VBuckets_PutRow(valueBuckets, valueBucket);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_VBuckets_PutRow:%{public}d", pt);

    char table[] = "test1";
    int64_t rowId = -1;

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int ret = OH_RdbTrans_BatchInsert(trans, table, valueBuckets, resolution, &rowId);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ERROR ? g_tSErrorCode : -1, &result);
    return result;
}

napi_value TestTransBatchInsertDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    OH_Data_VBuckets *valueBuckets = OH_VBuckets_Create();

    int pt = OH_VBuckets_PutRow(valueBuckets, valueBucket);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_VBuckets_PutRow:%{public}d", pt);

    char table[] = "test1";
    int64_t rowId = -1;

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int cl = OH_Rdb_CloseStore(testRdbStore);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_Rdb_CloseStore:%{public}d", cl);

    int ret = OH_RdbTrans_BatchInsert(trans, table, valueBuckets, resolution, &rowId);

    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}


napi_value TestUpdateInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    const char *table = "test";

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putText(valueBucket, "data1", "lisi");

    int64_t data2Value = 18;
    valueBucket->putInt64(valueBucket, "data2", data2Value);

    double data3Value = 12.5;
    valueBucket->putReal(valueBucket, "data3", data3Value);

    valueBucket->putNull(valueBucket, "data4");

    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table);

    OH_VObject *valueObject = OH_Rdb_CreateValueObject();

    const char *value = "ZhangSan";
    valueObject->putText(valueObject, value);

    predicates->equalTo(predicates, "data1", valueObject);

    int64_t changes = 0;
    int ret = OH_RdbTrans_Update(trans, valueBucket, predicates, &changes);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);

    predicates->destroy(predicates);
    valueObject->destroy(valueObject);
    valueBucket->destroy(valueBucket);

    return result;
}

napi_value TestTransUpdateError(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    char table[] = "test";
    int64_t rowId = -1;
    int res = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Insert:%{public}d", res);

    char table1[] = "test1";
    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table1);
    OH_VObject *valueObject = OH_Rdb_CreateValueObject();

    const char *value = "LiSi";
    valueObject->putText(valueObject, value);
    predicates->equalTo(predicates, "name", valueObject);

    int64_t changes = 0;

    int ret = OH_RdbTrans_Update(trans, valueBucket, predicates, &changes);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Update:%{public}d", ret);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    predicates->destroy(predicates);
    valueObject->destroy(valueObject);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ERROR ? g_tSErrorCode : -1, &result);
    return result;
}

napi_value TestTransUpdateDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "name TEXT, "
                                 "score INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "name", "ZhangSan");
    int64_t scoreValue = 99;
    valueBucket->putInt64(valueBucket, "score", scoreValue);

    char table[] = "test";
    int64_t rowId = -1;
    int res = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Insert:%{public}d", res);

    char table1[] = "test";
    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table1);
    OH_VObject *valueObject = OH_Rdb_CreateValueObject();

    const char *value = "LiSi";
    valueObject->putText(valueObject, value);
    predicates->equalTo(predicates, "name", valueObject);

    int64_t changes = 0;

    OH_Rdb_CloseStore(testRdbStore);

    int ret = OH_RdbTrans_Update(trans, valueBucket, predicates, &changes);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Update:%{public}d", ret);

    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    predicates->destroy(predicates);
    valueObject->destroy(valueObject);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestUpdateWithConflictResolutionInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    const char *table = "test";

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putText(valueBucket, "data1", "lisi");

    int64_t data2Value = 18;
    valueBucket->putInt64(valueBucket, "data2", data2Value);

    double data3Value = 12.5;
    valueBucket->putReal(valueBucket, "data3", data3Value);

    valueBucket->putNull(valueBucket, "data4");

    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table);

    OH_VObject *valueObject = OH_Rdb_CreateValueObject();

    const char *value = "ZhangSan";
    valueObject->putText(valueObject, value);

    predicates->equalTo(predicates, "data1", valueObject);

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int64_t changes = 0;
    int ret = OH_RdbTrans_UpdateWithConflictResolution(trans, valueBucket, predicates, resolution, &changes);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);

    predicates->destroy(predicates);
    valueObject->destroy(valueObject);
    valueBucket->destroy(valueBucket);

    return result;
}

napi_value TestTransUpdateWithConflictResolutionError(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 FLOAT, "
                                 "data3 INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    double data2Value = 99.9;
    valueBucket->putReal(valueBucket, "data2", data2Value);
    int64_t data3Value = 100;
    valueBucket->putInt64(valueBucket, "data3", data3Value);

    char table[] = "test";
    int64_t rowId = -1;
    int res = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Insert:%{public}d", res);

    char table1[] = "test1";
    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table1);
    OH_VObject *valueObject = OH_Rdb_CreateValueObject();

    const char *value = "LiSi";
    valueObject->putText(valueObject, value);
    predicates->equalTo(predicates, "data1", valueObject);

    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;

    int64_t changes = 0;

    int ret = OH_RdbTrans_UpdateWithConflictResolution(trans, valueBucket, predicates, resolution, &changes);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Update:%{public}d", ret);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    valueObject->destroy(valueObject);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ERROR ? g_tSErrorCode : -1, &result);
    return result;
}

napi_value TestTransUpdateWithConflictResolutionDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();
    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);
    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);
    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 FLOAT, "
                                 "data3 INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);
    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();
    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);
    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();
    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    double data2Value = 99.9;
    valueBucket->putReal(valueBucket, "data2", data2Value);
    int64_t data3Value = 100;
    valueBucket->putInt64(valueBucket, "data3", data3Value);
    char table[] = "test";
    int64_t rowId = -1;
    OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table);
    OH_VObject *valueObject = OH_Rdb_CreateValueObject();
    const char *value = "LiSi";
    valueObject->putText(valueObject, value);
    predicates->equalTo(predicates, "data1", valueObject);
    Rdb_ConflictResolution resolution = RDB_CONFLICT_NONE;
    int64_t changes = 0;
    OH_Rdb_CloseStore(testRdbStore);
    int ret = OH_RdbTrans_UpdateWithConflictResolution(trans, valueBucket, predicates, resolution, &changes);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    valueObject->destroy(valueObject);
    predicates->destroy(predicates);
    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestTransDeleteInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    const char *table = "test";

    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table);

    OH_VObject *valueObject = OH_Rdb_CreateValueObject();

    const char *value = "ZhangSan";
    valueObject->putText(valueObject, value);

    predicates->equalTo(predicates, "data1", valueObject);

    int64_t changes = 0;
    int ret = OH_RdbTrans_Delete(trans, predicates, &changes);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);

    predicates->destroy(predicates);
    valueObject->destroy(valueObject);

    return result;
}


napi_value TestTransDeleteError(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 FLOAT, "
                                 "data3 INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);
    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();
    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);
    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();
    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    double data2Value = 99.9;
    valueBucket->putReal(valueBucket, "data2", data2Value);
    int64_t data3Value = 100;
    valueBucket->putInt64(valueBucket, "data3", data3Value);
    char table[] = "test";
    int64_t rowId = -1;
    OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    char table1[] = "test1";
    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table1);
    OH_VObject *valueObject = OH_Rdb_CreateValueObject();
    const char *value = "ZhangSan";
    valueObject->putText(valueObject, value);
    predicates->equalTo(predicates, "data1", valueObject);
    int64_t changes = 0;
    int ret = OH_RdbTrans_Delete(trans, predicates, &changes);
    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    valueObject->destroy(valueObject);
    predicates->destroy(predicates);
    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ERROR ? g_tSErrorCode : -1, &result);
    return result;
}

napi_value TestTransDeleteDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();
    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);
    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);
    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 FLOAT, "
                                 "data3 INT64)";
    OH_Rdb_Execute(testRdbStore, createTableSql);
    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();
    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);
    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();
    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    double data2Value = 99.9;
    valueBucket->putReal(valueBucket, "data2", data2Value);
    int64_t data3Value = 100;
    valueBucket->putInt64(valueBucket, "data3", data3Value);
    char table[] = "test";
    int64_t rowId = -1;
    OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table);
    OH_VObject *valueObject = OH_Rdb_CreateValueObject();
    const char *value = "ZhangSan";
    valueObject->putText(valueObject, value);
    predicates->equalTo(predicates, "data1", valueObject);
    int64_t changes = 0;
    OH_Rdb_CloseStore(testRdbStore);
    int ret = OH_RdbTrans_Delete(trans, predicates, &changes);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    valueObject->destroy(valueObject);
    predicates->destroy(predicates);
    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestTransExecuteInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    const char *insertSql = "INSERT INTO test (data1) VALUES (?)";

    OH_Data_Values *values = OH_Values_Create();

    OH_Values_PutInt(values, 10);

    OH_Data_Value *outValue = nullptr;

    int ret = OH_RdbTrans_Execute(trans, insertSql, values, &outValue);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);
    OH_Values_Destroy(values);
    return result;
}

napi_value TestTransExecuteError(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 FLOAT, "
                                 "data3 INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    double data2Value = 99.9;
    valueBucket->putReal(valueBucket, "data2", data2Value);
    int64_t data3Value = 100;
    valueBucket->putInt64(valueBucket, "data3", data3Value);

    char table[] = "test";
    int64_t rowId = -1;
    int res = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Insert:%{public}d", res);

    const char *insertSql = "INSERT INTO test1 (data3) VALUES (?)";

    OH_Data_Values *values = OH_Values_Create();
    int64_t val = 10;
    OH_Values_PutInt(values, val);
    OH_Data_Value *outValue = 0;

    int ret = OH_RdbTrans_Execute(trans, insertSql, values, &outValue);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Execute:%{public}d", ret);

    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    OH_Values_Destroy(values);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ERROR ? g_tSErrorCode : -1, &result);
    return result;
}

napi_value TestTransExecuteDatabaseClosed(napi_env env, napi_callback_info info) {
    napi_value result;

    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();

    OH_Rdb_SetDatabaseDir(configV2, TransactionTest::g_rdbTest);
    OH_Rdb_SetStoreName(configV2, TransactionTest::g_rdbStoreName);
    OH_Rdb_SetBundleName(configV2, TransactionTest::g_bundleName);
    OH_Rdb_SetModuleName(configV2, TransactionTest::g_moduleName);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);

    OH_Rdb_Store *testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);

    const char *createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 FLOAT, "
                                 "data3 INT64)";

    OH_Rdb_Execute(testRdbStore, createTableSql);

    OH_Rdb_Transaction *trans = nullptr;
    OH_RDB_TransOptions *options = OH_RdbTrans_CreateOptions();

    OH_Rdb_CreateTransaction(testRdbStore, options, &trans);

    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();

    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    double data2Value = 99.9;
    valueBucket->putReal(valueBucket, "data2", data2Value);
    int64_t data3Value = 100;
    valueBucket->putInt64(valueBucket, "data3", data3Value);

    char table[] = "test";
    int64_t rowId = -1;
    int res = OH_RdbTrans_Insert(trans, table, valueBucket, &rowId);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Insert:%{public}d", res);

    const char *insertSql = "INSERT INTO test (data3) VALUES (?)";

    OH_Data_Values *values = OH_Values_Create();
    int64_t val = 10;
    OH_Values_PutInt(values, val);
    OH_Data_Value *outValue = 0;

    OH_Rdb_CloseStore(testRdbStore);

    int ret = OH_RdbTrans_Execute(trans, insertSql, values, &outValue);

    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_RdbTrans_Execute:%{public}d", ret);

    OH_Rdb_DeleteStoreV2(configV2);
    OH_RdbTrans_Destroy(trans);
    OH_RdbTrans_DestroyOptions(options);
    valueBucket->destroy(valueBucket);
    OH_Values_Destroy(values);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_ALREADY_CLOSED ? g_tSAlreadyClosedCode : -1, &result);
    return result;
}

napi_value TestDestroyInvalidParam(napi_env env, napi_callback_info info) {
    napi_value result;
    OH_Rdb_Transaction *trans = nullptr;

    int ret = OH_RdbTrans_Destroy(trans);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_ARGS ? g_tSInvalidParamCode : -1, &result);
    return result;
}