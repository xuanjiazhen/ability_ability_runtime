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

#include "RdbErrorCodeTest.h"
#include "napi/native_api.h"
#include <cstddef>
#include <database/rdb/relational_store.h>
#include <js_native_api_types.h>
#include <napi/native_api.h>
#include <database/rdb/oh_values_bucket.h>
#include <database/rdb/relational_store_error_code.h>
#include <hilog/log.h>

namespace RdbErrorCodeTest {
    const char* RDB_TEST_PATH = "/data/storage/el2/database/com.acts.acttransactiontest/";
    char RDB_STORE_NAME[] = "rdb_store_test.db";
    char BUNDLE_NAME[] = "com.acts.acttransactiontest";
    char MODULE_NAME[] = "";
    OH_Rdb_Config config_; 
}

static void InitRdbConfig()
{
    RdbErrorCodeTest::config_.dataBaseDir = RdbErrorCodeTest::RDB_TEST_PATH;
    RdbErrorCodeTest::config_.storeName = RdbErrorCodeTest::RDB_STORE_NAME;
    RdbErrorCodeTest::config_.bundleName = RdbErrorCodeTest::BUNDLE_NAME;
    RdbErrorCodeTest::config_.moduleName = RdbErrorCodeTest::MODULE_NAME;
    RdbErrorCodeTest::config_.securityLevel = OH_Rdb_SecurityLevel::S1;
    RdbErrorCodeTest::config_.isEncrypt = false;
    RdbErrorCodeTest::config_.selfSize = sizeof(OH_Rdb_Config);
}

void addRdbErrorCodeTest(std::vector<napi_property_descriptor> &descData)
{
    descData.push_back({"testRdbError", nullptr, TestRdbError, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back(
        {"testEmptyTableName", nullptr, TestEmptyTableName, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back(
        {"testInvalidColumnType", nullptr, TestInvalidColumnType, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back(
        {"testNoRowInQuery", nullptr, TestNoRowInQuery, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testNoTransactionInSession", nullptr, TestNoTransactionInSession, nullptr, nullptr, nullptr,
                        napi_default, nullptr});
}

napi_value TestRdbError(napi_env env, napi_callback_info info)
{
    napi_value result;
    InitRdbConfig();
    
    OH_Rdb_Store *testRdbStore = nullptr;
    
    int errCode = 0;
    char table[] = "test"; 
    
    testRdbStore = OH_Rdb_GetOrOpen(&RdbErrorCodeTest::config_, &errCode); 
    
    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();
    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    valueBucket->putInt64(valueBucket, "data2", 12000);
    valueBucket->putReal(valueBucket, "data3", 12.15);
    
    int ret = OH_Rdb_Insert(testRdbStore, table, valueBucket);

    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_ERR ? -1 : 1, &result);
    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStore(&RdbErrorCodeTest::config_);
    valueBucket->destroy(valueBucket);
    return result;
}

napi_value TestEmptyTableName(napi_env env, napi_callback_info info)
{
    napi_value result;
    
    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();
    
    OH_Rdb_SetDatabaseDir(configV2, RdbErrorCodeTest::RDB_TEST_PATH);
    OH_Rdb_SetStoreName(configV2, RdbErrorCodeTest::RDB_STORE_NAME);
    OH_Rdb_SetBundleName(configV2, RdbErrorCodeTest::BUNDLE_NAME);
    OH_Rdb_SetModuleName(configV2, RdbErrorCodeTest::MODULE_NAME);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_CAYLEY );
    
    OH_Rdb_Store * store = nullptr;
    int errCode = 0;
    store = OH_Rdb_CreateOrOpen(configV2, &errCode);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_CreateOrOpen:%{public}d", errCode);
    
    const char* createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 INT64, "
                                 "data3 REAL)";

    int ree = OH_Rdb_Execute(store, createTableSql);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_Execute:%{public}d", ree);
    
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("");
    int ret = OH_Rdb_LockRow(store, predicates);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_LockRow:%{public}d", ret);
    
    OH_Rdb_CloseStore(store);
    OH_Rdb_DeleteStoreV2(configV2);
    predicates->destroy(predicates);
    
    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_EMPTY_TABLE_NAME ? 14800005 : -1, &result);
    
    return result;
}

napi_value TestInvalidColumnType(napi_env env, napi_callback_info info)
{
    napi_value result;
    
    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();
    
    OH_Rdb_SetDatabaseDir(configV2, RdbErrorCodeTest::RDB_TEST_PATH);
    OH_Rdb_SetStoreName(configV2, RdbErrorCodeTest::RDB_STORE_NAME);
    OH_Rdb_SetBundleName(configV2, RdbErrorCodeTest::BUNDLE_NAME);
    OH_Rdb_SetModuleName(configV2, RdbErrorCodeTest::MODULE_NAME);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_CAYLEY );
    
    OH_Rdb_Store * store = nullptr;
    int errCode = 0;
    store = OH_Rdb_CreateOrOpen(configV2, &errCode);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_CreateOrOpen:%{public}d", errCode);
    
    const char* createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 INT64, "
                                 "data3 REAL)";

    int ree = OH_Rdb_Execute(store, createTableSql);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_Execute:%{public}d", ree);
    
    OH_VBucket *valueBucket = OH_Rdb_CreateValuesBucket();
    
    valueBucket->putInt64(valueBucket, "id", 1);
    valueBucket->putText(valueBucket, "data1", "ZhangSan");
    valueBucket->putInt64(valueBucket, "data2", 12800);
    valueBucket->putReal(valueBucket, "data3", 128.0);
    
    int ret1 = OH_Rdb_Insert(store, "test", valueBucket);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_Insert:%{public}d", ret1);
    
    const char *sql = "SELECT * FROM test;";
    
    OH_Data_Values *values = OH_Values_Create();
    
    OH_Cursor *cursor = OH_Rdb_ExecuteQueryV2(store, sql, values);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_ExecuteQueryV2:%{public}s", (char*) cursor);
    
    cursor->goToNextRow(cursor);
    
    int cnt = 0;
    cursor->getColumnCount(cursor, &cnt);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  getColumnCount:%{public}d", cnt);
    
    int rowCnt = 0;
    cursor->getRowCount(cursor, &rowCnt);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  getRowCount:%{public}d", rowCnt);
    
    int columnIndex = 0;
    const char *name = "data3";
    int rei = cursor->getColumnIndex(cursor, name, &columnIndex);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  getColumnIndexcnt:%{public}d", columnIndex);
    
    OH_ColumnType type;
    cursor->getColumnType(cursor, columnIndex, &type);
    
    const char *typeStr;
    switch (type) {
    case TYPE_INT64:
        typeStr = "TYPE_INT64";
        break;
    case TYPE_REAL:
        typeStr = "TYPE_REAL";
        break;
    case TYPE_TEXT:
        typeStr = "TYPE_TEXT";
        break;
    default:
        typeStr = "unKnown";
        break;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  getColumnType:%{public}s", typeStr);
    
    size_t size = 0;
    int ret = cursor->getSize(cursor, columnIndex, &size);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  getSize:%{public}d", ret);
    
    OH_Rdb_CloseStore(store);
    OH_Rdb_DeleteStoreV2(configV2);
    valueBucket->destroy(valueBucket);
    OH_Values_Destroy(values);
    cursor->destroy(cursor);
    
    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_INVALID_COLUMN_TYPE ? 14800009 : 1, &result);
    
    return result;
}

napi_value TestNoRowInQuery(napi_env env, napi_callback_info info)
{
    napi_value result;
    
    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();
    
    OH_Rdb_SetDatabaseDir(configV2, RdbErrorCodeTest::RDB_TEST_PATH);
    OH_Rdb_SetStoreName(configV2, RdbErrorCodeTest::RDB_STORE_NAME);
    OH_Rdb_SetBundleName(configV2, RdbErrorCodeTest::BUNDLE_NAME);
    OH_Rdb_SetModuleName(configV2, RdbErrorCodeTest::MODULE_NAME);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_CAYLEY );
    
    OH_Rdb_Store * store = nullptr;
    int errCode = 0;
    store = OH_Rdb_CreateOrOpen(configV2, &errCode);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_CreateOrOpen:%{public}d", errCode);
    
    const char* createTableSql = "CREATE TABLE IF NOT EXISTS test ("
                                 "id INT64 PRIMARY KEY, "
                                 "data1 TEXT, "
                                 "data2 INT64, "
                                 "data3 REAL)";

    int ree = OH_Rdb_Execute(store, createTableSql);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_Execute:%{public}d", ree);
    
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    int ret = OH_Rdb_LockRow(store, predicates);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkWeb", "=======zzzhh  OH_Rdb_LockRow:%{public}d", ret);
    
    OH_Rdb_CloseStore(store);
    OH_Rdb_DeleteStoreV2(configV2);
    predicates->destroy(predicates);
    
    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_NO_ROW_IN_QUERY ? 14800018 : -1, &result);
    
    return result;
}


napi_value TestNoTransactionInSession(napi_env env, napi_callback_info info)
{
    napi_value result;
    
    OH_Rdb_ConfigV2 *configV2 = OH_Rdb_CreateConfig();
    
    OH_Rdb_SetDatabaseDir(configV2, RdbErrorCodeTest::RDB_TEST_PATH);
    OH_Rdb_SetStoreName(configV2, RdbErrorCodeTest::RDB_STORE_NAME);
    OH_Rdb_SetBundleName(configV2, RdbErrorCodeTest::BUNDLE_NAME);
    OH_Rdb_SetModuleName(configV2, RdbErrorCodeTest::MODULE_NAME);
    OH_Rdb_SetEncrypted(configV2, false);
    OH_Rdb_SetSecurityLevel(configV2, OH_Rdb_SecurityLevel::S1);
    OH_Rdb_SetArea(configV2, RDB_SECURITY_AREA_EL1);
    OH_Rdb_SetDbType(configV2, RDB_SQLITE);
    
    OH_Rdb_Store * testRdbStore = nullptr;
    int errCode = 0;
    testRdbStore = OH_Rdb_CreateOrOpen(configV2, &errCode);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_Rdb_GetOrOpen:%{public}s", (char *)testRdbStore);
    
    int ret = OH_Rdb_RollBack(testRdbStore);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkData", "=======zh  OH_Rdb_RollBack:%{public}d", ret);
    
    OH_Rdb_CloseStore(testRdbStore);
    OH_Rdb_DeleteStoreV2(configV2);
    OH_Rdb_DestroyConfig(configV2);
    
    napi_create_int32(env, ret == OH_Rdb_ErrCode::RDB_E_NO_TRANSACTION_IN_SESSION ? 14800016 : -1, &result);
    return result;
}