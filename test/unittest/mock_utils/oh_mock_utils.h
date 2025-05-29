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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include <string>
#include <typeinfo>
#include <map>
#include <vector>
#include <any>
#include <typeinfo>

class ResultWrap {
public:
    ResultWrap(std::any result)
    {
        result_ = result;
    }

    ResultWrap() {}

    template <typename T>
    T Get() const
    {
        return std::any_cast<T>(result_);
    }
private:
    std::any result_;
};

std::map<std::string, std::vector<ResultWrap>> g_mockMap;

// return value only support basic type or type with default constructor
#define OH_MOCK_STATIC_METHOD(ret, className, funcName, ...)                       \
static ret funcName(__VA_ARGS__)                                                   \
{                                                                                  \
    std::string key = #className"_"#funcName"_"#__VA_ARGS__;                       \
    if (g_mockMap.find(key) == g_mockMap.end()) {                                  \
        g_mockMap[key] = {};                                                       \
    }                                                                              \
    ResultWrap tempRet;                                                                   \
    auto it = g_mockMap.find(key);                                                 \
    if (it != g_mockMap.end()) {                                                   \
        std::vector<ResultWrap> expectRets = it->second;                                 \
        if (!expectRets.empty()) {                                                 \
            tempRet = expectRets[0];                                               \
            expectRets.erase(expectRets.begin());                                  \
        }                                                                          \
        g_mockMap[key] = expectRets;                                               \
    }                                                                              \
    return tempRet.Get<ret>();                                                                \
}

// return value only support basic type or type with default constructor
#define OH_MOCK_GLOBAL_STATIC_METHOD(ret, funcName, ...)                       \
static ret funcName(__VA_ARGS__)                                 \
{                                                                                  \
    std::string key = #funcName"_"#__VA_ARGS__; \
    if (g_mockMap.find(key) == g_mockMap.end()) {                                  \
        std::vector<ResultWrap> tempExpectRets;                                        \
        g_mockMap[key] = tempExpectRets;                                 \
    }                                                                              \
    ResultWrap tempRet;                                                                 \
    std::vector<ResultWrap>& expectRets = g_mockMap[key];                       \
    if (!expectRets.empty()) {                                                     \
        tempRet = expectRets[0];                                          \
        expectRets.erase(expectRets.begin());                                      \
    }                                                                              \
    return tempRet.Get<ret>();                                                                \
}

#define OH_MOCK_METHOD_EXPECT_RET(expectRetVec, className, funcName,...)           \
do {                                                                               \
    std::string key = #className"_"#funcName"_"#__VA_ARGS__; \
    std::vector<ResultWrap> v; \
    for (auto e : expectRetVec) { \
        ResultWrap r(e); \
        v.emplace_back(r); \
    } \
    g_mockMap[key] = v;                                                 \
} while (0)

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
