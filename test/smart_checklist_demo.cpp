/**
 * Smart Checklist Demo
 * Testing the improved checklist generation
 */

#include <iostream>
#include <string>
#include <vector>

class SafeStringProcessor {
public:
    // Safe string processing with proper initialization
    SafeStringProcessor() : initialized_(false) {}

    // Process string with bounds checking
    bool Process(const std::string& input) {
        if (input.empty()) {
            return false;
        }

        // Use size_t for loop variable (fixed width type)
        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == ' ') {
                space_count_++;
            }
        }

        initialized_ = true;
        return true;
    }

    // Safe getter with const reference
    size_t GetSpaceCount() const {
        return space_count_;
    }

private:
    bool initialized_;
    size_t space_count_;
};

int main() {
    SafeStringProcessor processor;

    std::string test = "Hello World Test";
    if (processor.Process(test)) {
        std::cout << "Space count: " << processor.GetSpaceCount() << std::endl;
    }

    return 0;
}
