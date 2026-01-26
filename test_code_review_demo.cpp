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

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>

/**
 * @brief Code review demo class demonstrating various C++ patterns
 *
 * This class demonstrates both correct and potentially problematic patterns
 * for testing the gitcode-automation skill's code review capabilities.
 */
class CodeReviewDemo {
public:
    CodeReviewDemo() : data_count_(0), is_initialized_(false) {
        std::cout << "CodeReviewDemo created" << std::endl;
    }

    ~CodeReviewDemo() {
        std::cout << "CodeReviewDemo destroyed" << std::endl;
    }

    /**
     * @brief Process data with shared pointer (correct pattern)
     *
     * Analysis:
     * - Uses std::make_shared (correct)
     * - No raw pointer conversion
     * - Clear ownership
     */
    void ProcessData(const std::string& input) {
        // ✓ Correct: Using make_shared instead of raw pointer
        auto data = std::make_shared<std::string>(input);
        ProcessDataInternal(data);
    }

    /**
     * @brief Thread-safe data access (correct pattern)
     *
     * Analysis:
     * - Uses mutex lock for thread safety
     * - Lock scope is appropriate
     * - No lock in signal handler
     */
    void AddDataItem(const std::string& item) {
        // ✓ Correct: Proper locking for concurrent access
        std::lock_guard<std::mutex> lock(mutex_);
        data_items_.push_back(item);
        data_count_++;
    }

    /**
     * @brief Lambda with reference capture (needs review)
     *
     * Analysis:
     * - Lambda captures 'this' by reference (implicit)
     * - Potential dangling reference if object destroyed
     * - Better to capture by value or use smart pointer
     */
    void ScheduleDelayedTask(int delay_ms) {
        // ⚠ Review needed: Lambda with implicit 'this' capture
        // If object destroyed before callback executes, this becomes invalid
        std::thread([this, delay_ms]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            this->ExecuteTask();  // Potential use-after-free
        }).detach();
    }

    /**
     * @brief STL container usage (needs concurrent access check)
     *
     * Analysis:
     * - Using std::vector
     * - Thread safety depends on how it's accessed
     * - AddDataItem uses mutex, but GetItem doesn't
     */
    std::string GetItem(size_t index) const {
        // ⚠ Review needed: No lock protection for read
        // If AddDataItem called concurrently, vector could be reallocated
        if (index < data_items_.size()) {
            return data_items_[index];
        }
        return "";
    }

    /**
     * @brief Member variable initialization check
     *
     * Analysis:
     * - data_count_: Initialized (✓)
     * - is_initialized_: Initialized (✓)
     * - All member variables have initializers
     */
    bool IsInitialized() const {
        return is_initialized_;
    }

    /**
     * @brief External string usage (correct pattern)
     *
     * Analysis:
     * - Passed by const reference (✓)
     * - Copied before storage (✓)
     * - No lifetime issues
     */
    void SetExternalData(const std::string& external_data) {
        // ✓ Correct: Copying external string before use
        external_data_ = external_data;
        is_initialized_ = true;
    }

    /**
     * @brief Multiple smart pointers managing same address (demonstrates anti-pattern)
     *
     * Analysis:
     * - This is commented out to demonstrate what NOT to do
     * - Never create independent smart pointers from same raw pointer
     */
    /*
    void AntiPatternExample() {
        // ✗ WRONG: Multiple independent smart pointers managing same address
        int* raw = new int(42);
        auto ptr1 = std::shared_ptr<int>(raw);
        auto ptr2 = std::shared_ptr<int>(raw);  // Double free when both destroyed!
    }
    */

    /**
     * @brief Correct smart pointer usage
     *
     * Analysis:
     * - Single shared_ptr created with make_shared (✓)
     * - Clear ownership semantics (✓)
     */
    void CorrectPatternExample() {
        // ✓ Correct: Using make_shared
        auto ptr = std::make_shared<int>(42);
        ProcessInteger(ptr);
    }

private:
    void ProcessDataInternal(std::shared_ptr<std::string> data) {
        std::cout << "Processing: " << *data << std::endl;
    }

    void ProcessInteger(std::shared_ptr<int> value) {
        std::cout << "Value: " << *value << std::endl;
    }

    void ExecuteTask() {
        std::cout << "Task executed" << std::endl;
    }

    // ✓ All member variables initialized
    size_t data_count_;
    bool is_initialized_;
    std::vector<std::string> data_items_;
    std::string external_data_;
    std::mutex mutex_;
};

// Global instance for testing (not thread-safe access)
CodeReviewDemo* g_demo_instance = nullptr;

/**
 * @brief Initialization function
 *
 * Analysis:
 * - Uses new/delete instead of smart pointer (⚠ review needed)
 * - Global variable without mutex protection (⚠ review needed)
 */
void InitializeDemo() {
    if (!g_demo_instance) {
        // ⚠ Review needed: Using raw pointer for global
        g_demo_instance = new CodeReviewDemo();
    }
}

/**
 * @brief Cleanup function
 *
 * Analysis:
 * - Matches allocation (✓)
 * - No exception safety (⚠ review needed)
 */
void CleanupDemo() {
    delete g_demo_instance;
    g_demo_instance = nullptr;
}

int main() {
    std::cout << "=== Code Review Automation Demo ===" << std::endl;

    // Test basic functionality
    CodeReviewDemo demo;
    demo.SetExternalData("Test data");
    demo.ProcessData("Input data");
    demo.AddDataItem("Item 1");
    demo.AddDataItem("Item 2");

    std::cout << "Item 0: " << demo.GetItem(0) << std::endl;
    std::cout << "Initialized: " << demo.IsInitialized() << std::endl;

    std::cout << "=== Demo Complete ===" << std::endl;
    return 0;
}
