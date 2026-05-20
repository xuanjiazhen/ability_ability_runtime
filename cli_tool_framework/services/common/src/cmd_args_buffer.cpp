/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cmd_args_buffer.h"

namespace OHOS {
namespace CliTool {

void CmdArgsBuffer::AddParam(const std::string& key, const std::string& value)
{
    std::string fullKey = "--" + key;
    strings_.push_back(std::move(fullKey));
    strings_.push_back(value);
    UpdatePointers();
}

void CmdArgsBuffer::AddFlag(const std::string& key)
{
    std::string fullKey = "--" + key;
    strings_.push_back(std::move(fullKey));
    UpdatePointers();
}

void CmdArgsBuffer::AddArg(const std::string& arg)
{
    strings_.push_back(arg);
    UpdatePointers();
}

char** CmdArgsBuffer::data()
{
    return pointers_.data();
}

const char* const* CmdArgsBuffer::data() const
{
    return pointers_.data();
}

size_t CmdArgsBuffer::size() const
{
    return pointers_.size();
}

char* CmdArgsBuffer::operator[](size_t index)
{
    return pointers_[index];
}

const char* CmdArgsBuffer::operator[](size_t index) const
{
    return pointers_[index];
}

void CmdArgsBuffer::clear()
{
    strings_.clear();
    pointers_.clear();
    UpdatePointers();
}

void CmdArgsBuffer::reserve(size_t capacity)
{
    strings_.reserve(capacity);
    pointers_.reserve(capacity + 1); // +1 for null terminator
}

void CmdArgsBuffer::UpdatePointers()
{
    pointers_.clear();
    pointers_.reserve(strings_.size() + 1); // +1 for null terminator

    for (auto& str : strings_) {
        pointers_.push_back(const_cast<char*>(str.c_str()));
    }
    pointers_.push_back(nullptr); // Null-terminate for execvp
}

} // namespace CliTool
} // namespace OHOS
