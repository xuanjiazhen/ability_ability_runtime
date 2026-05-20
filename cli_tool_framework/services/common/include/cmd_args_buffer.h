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

#ifndef OHOS_ABILITY_RUNTIME_CMD_ARGS_BUFFER_H
#define OHOS_ABILITY_RUNTIME_CMD_ARGS_BUFFER_H

#include <vector>
#include <string>

namespace OHOS {
namespace CliTool {

/**
 * @brief Buffer class to manage string lifetime for command line arguments
 *
 * This class stores the actual string data and provides pointers to that data.
 * The strings are guaranteed to remain valid as long as this object exists.
 *
 * This solves the dangling pointer problem when converting parameters to
 * command line arguments for execvp() and similar functions.
 *
 * Usage:
 *   CmdArgsBuffer buffer;
 *   buffer.AddArg("/usr/bin/command");
 *   buffer.AddParam("verbose", "true");
 *   buffer.AddFlag("help");
 *   execvp(buffer[0], buffer.data());
 */
class CmdArgsBuffer {
public:
    CmdArgsBuffer() = default;

    // Disable copy to prevent accidental duplication of internal state
    CmdArgsBuffer(const CmdArgsBuffer&) = delete;
    CmdArgsBuffer& operator=(const CmdArgsBuffer&) = delete;

    // Enable move semantics for efficient transfer
    CmdArgsBuffer(CmdArgsBuffer&&) = default;
    CmdArgsBuffer& operator=(CmdArgsBuffer&&) = default;

    ~CmdArgsBuffer() = default;

    /**
     * @brief Add a key-value pair to the buffer
     * @param key The parameter key (will be prefixed with "--")
     * @param value The parameter value
     *
     * Example: AddParam("output", "file.txt") → adds "--output" and "file.txt"
     */
    void AddParam(const std::string& key, const std::string& value);

    /**
     * @brief Add a flag (key only) to the buffer
     * @param key The flag name (will be prefixed with "--")
     *
     * Example: AddFlag("verbose") → adds "--verbose"
     */
    void AddFlag(const std::string& key);

    /**
     * @brief Add a raw string argument to the buffer (no prefix added)
     * @param arg The argument string
     *
     * Use this for executable paths, subcommands, or arguments that
     * should not have the "--" prefix.
     *
     * Example: AddArg("/usr/bin/git") → adds "/usr/bin/git"
     */
    void AddArg(const std::string& arg);

    /**
     * @brief Get the pointer array for use with execvp etc.
     * @return Pointer to null-terminated array of char pointers
     *
     * The returned array is null-terminated, suitable for execvp().
     */
    char** data();

    /**
     * @brief Get the pointer array for use with execvp etc. (const version)
     * @return Pointer to null-terminated array of const char pointers
     */
    const char* const* data() const;

    /**
     * @brief Get the number of arguments (excluding the null terminator)
     * @return Number of argument pointers
     */
    size_t size() const;

    /**
     * @brief Access individual arguments by index
     * @param index The index of the argument
     * @return Pointer to the argument string
     *
     * Note: The index must be valid (less than size()).
     * The returned pointer points to internal data and remains
     * valid as long as this CmdArgsBuffer object exists.
     */
    char* operator[](size_t index);

    /**
     * @brief Access individual arguments by index (const version)
     * @param index The index of the argument
     * @return Const pointer to the argument string
     */
    const char* operator[](size_t index) const;

    /**
     * @brief Clear all arguments from the buffer
     *
     * After calling this, size() returns 0 and data() returns a pointer
     * to an array containing only the null terminator.
     */
    void clear();

    /**
     * @brief Reserve space for a specific number of arguments
     * @param capacity The number of arguments to reserve space for
     *
     * This can improve performance when adding many arguments,
     * as it reduces the number of reallocations.
     */
    void reserve(size_t capacity);

private:
    /**
     * @brief Update the pointers array after modifying strings
     *
     * This function rebuilds the pointers array to ensure all
     * pointers point to the correct string data.
     */
    void UpdatePointers();

    std::vector<std::string> strings_;   ///< Stores the actual string data
    std::vector<char*> pointers_;        ///< Pointers to strings (null-terminated)
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CMD_ARGS_BUFFER_H
