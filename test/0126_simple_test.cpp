/**
 * Simple test file for branch 0126
 * Created: 2026-01-26
 * Purpose: Test PR creation to OpenHarmony master
 */

#include <iostream>
#include <string>

// Simple utility function for demonstration
std::string GetGreeting(const std::string& name) {
    return "Hello, " + name + "! Welcome to ability_runtime.";
}

// Test function
void TestFeature0126() {
    std::cout << "Testing feature 0126..." << std::endl;
    std::cout << GetGreeting("OpenHarmony") << std::endl;

    // Simple calculation
    int sum = 0;
    for (int i = 1; i <= 10; i++) {
        sum += i;
    }
    std::cout << "Sum of 1 to 10: " << sum << std::endl;
}

int main() {
    TestFeature0126();
    return 0;
}
