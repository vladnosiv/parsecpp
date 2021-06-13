#include <iostream>

#include "Test.hpp"

bool ::Testing::Testable::check(bool condition, std::string fileName,
                                std::string expr, std::string functionName, int line) {
    if (!condition) {
        std::cerr << "Assertion failed! " << expr <<
                  " in " << fileName << ":" << line <<
                  " " << functionName << std::endl;
        failed = true;
    }
    return condition;
}