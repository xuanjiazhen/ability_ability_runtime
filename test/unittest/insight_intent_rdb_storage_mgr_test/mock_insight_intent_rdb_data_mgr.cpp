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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "insight_intent_rdb_data_mgr.h"

namespace OHOS {
namespace AbilityRuntime {
bool g_mockQueryDataRet = true;
bool g_mockQueryDataBeginWithKeyRet = true;
bool g_mockInsertDataRet = true;
bool g_mockBatchInsertDataRet = true;
bool g_mockBatchUpdateDataRet = true;
bool g_mockBatchDeleteDataRet = true;
bool g_mockBatchDeleteDataBeginWithKeyRet = true;
bool g_mockBatchReplaceDataRet = true;
bool g_mockDeleteDataRet = true;
bool g_mockDeleteDataBeginWithKeyRet = true;
std::string g_mockQueryDataKey = "";
std::unordered_map<std::string, std::string> g_mockQueryDatas;
bool g_mockDeleteDataBeginWithKeyCalled = false;

void MockQueryData(bool mockRet)
{
    g_mockQueryDataRet = mockRet;
}


void MockQueryDataBeginWithKey(bool mockRet)
{
    g_mockQueryDataBeginWithKeyRet = mockRet;
}

void MockQueryDataBeginWithKeyData(const std::string &key, const std::string &value)
{
    g_mockQueryDatas.emplace(key, value);
}

void MockQueryDataBeginWithKeyDataClear()
{
    g_mockQueryDatas.clear();
}

std::string GetMockQueryDataKey()
{
    return g_mockQueryDataKey;
}

void MockDeleteDataBeginWithKeyCalledReset()
{
    g_mockDeleteDataBeginWithKeyCalled = false;
}

bool GetMockDeleteDataBeginWithKeyCalled()
{
    return g_mockDeleteDataBeginWithKeyCalled;
}

void MockInsertData(bool mockRet)
{
    g_mockInsertDataRet = mockRet;
}

void MockBatchInsertData(bool mockRet)
{
    g_mockBatchInsertDataRet = mockRet;
}

void MockBatchReplaceData(bool mockRet)
{
    g_mockBatchReplaceDataRet = mockRet;
}

void MockBatchUpdateData(bool mockRet)
{
    g_mockBatchUpdateDataRet = mockRet;
}

void MockBatchDeleteData(bool mockRet)
{
    g_mockBatchDeleteDataRet = mockRet;
}

void MockBatchDeleteDataBeginWithKey(bool mockRet)
{
    g_mockBatchDeleteDataBeginWithKeyRet = mockRet;
}

void MockDeleteData(bool mockRet)
{
    g_mockDeleteDataRet = mockRet;
}

void MockDeleteDataBeginWithKey(bool mockRet)
{
    g_mockDeleteDataBeginWithKeyRet = mockRet;
}

}
}

namespace OHOS {
namespace AbilityRuntime {
InsightIntentRdbDataMgr::InsightIntentRdbDataMgr()
{}

InsightIntentRdbDataMgr::~InsightIntentRdbDataMgr()
{}

bool InsightIntentRdbDataMgr::QueryData(const std::string &key, std::string &value)
{
    if (g_mockQueryDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::QueryDataBeginWithKey(const std::string &key,
    std::unordered_map<std::string, std::string> &datas)
{
    g_mockQueryDataKey = key;
    if (!g_mockQueryDataBeginWithKeyRet) {
        return false;
    }
    for (const auto &item : g_mockQueryDatas) {
        if (item.first.compare(0, key.size(), key) == 0) {
            datas.emplace(item.first, item.second);
        }
    }
    return true;
}

bool InsightIntentRdbDataMgr::InsertData(const std::string &key, const std::string &value)
{
    if (g_mockInsertDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::BatchInsertData(const std::vector<std::pair<std::string, std::string>> &keyValues)
{
    if (g_mockBatchInsertDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::BatchReplaceData(const std::vector<std::string> &deleteKeyPrefixes,
    const std::vector<std::pair<std::string, std::string>> &keyValues)
{
    if (g_mockBatchReplaceDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::BatchUpdateData(const std::vector<std::pair<std::string, std::string>> &keyValues)
{
    if (g_mockBatchUpdateDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::BatchDeleteData(const std::vector<std::string> &keys)
{
    if (g_mockBatchDeleteDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::BatchDeleteDataBeginWithKey(const std::vector<std::string> &keyPrefixes)
{
    if (g_mockBatchDeleteDataBeginWithKeyRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::DeleteData(const std::string &key)
{
    if (g_mockDeleteDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::DeleteDataBeginWithKey(const std::string &key)
{
    g_mockDeleteDataBeginWithKeyCalled = true;
    if (g_mockDeleteDataBeginWithKeyRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::UpdateData(const std::string &key, const std::string &value)
{
    return true;
}

bool InsightIntentRdbDataMgr::QueryAllData(std::unordered_map<std::string, std::string> &datas)
{
    return true;
}

void InsightIntentRdbDataMgr::BackupRdb()
{
    return;
}

IntentRdbOpenCallback::IntentRdbOpenCallback(const IntentRdbConfig &intentRdbConfig)
{
}

int32_t IntentRdbOpenCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnUpgrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnDowngrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::onCorruption(std::string databaseFile)
{
    return NativeRdb::E_OK;
}
}
}