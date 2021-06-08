#include "Parser.hpp"

#include <string>
#include <iostream>
#include <optional>

int main() {
    using namespace Parser;
    auto ch = spaces() >> seq(alphaNum(), charParser('+'));
    std::string str = "     9+1+2+3+5";
    auto result = ch.eval(str);
    if (result.has_value()) {
        std::cout << "Parsed\n";
        auto v = result.value().first;
        for (char c : v) {
            std::cout << c << ' ';
        }
    } else {
        std::cout << str << " Not parsed" << std::endl;
    }
}