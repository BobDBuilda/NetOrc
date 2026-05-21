#include <iostream>
#include <vector>
#include <numeric>

int main() {
    // 1. Basic Inline Execution
    // Similar to (function() { ... })() in JS
    []() {
        std::cout << "1. Basic inline execution running!" << std::endl;
    }();

    // 2. Complex Const Initialization
    // This is a common C++ pattern to keep variables 'const' even if 
    // their initialization logic is complex.
    const int complex_value = []() {
        int sum = 0;
        for (int i = 0; i < 10; ++i) {
            sum += i;
        }
        return sum;
    }();
    std::cout << "2. Complex const value: " << complex_value << std::endl;

    // 3. Captures and Arguments
    // Capturing 'factor' and passing 'input' as an argument
    int factor = 10;
    int input = 5;

    int result = [factor](int val) {
        return factor * val;
    }(input);
    std::cout << "3. Result of capture and argument: " << result << std::endl;

    return 0;
}
