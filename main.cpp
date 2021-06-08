#include "Parser.hpp"

#include <string>
#include <iostream>
#include <optional>

int main() {
    using namespace Parser;
    auto ch = spaces() >> alphaNum();
    std::string str = "       1abc";
    auto result = ch.eval(str);
    if (result.has_value()) {
        std::cout << '\"' << str << '\"' << " Parsed. Right side: \"" << result.value().second << '\"' << std::endl;
    } else {
        std::cout << str << " Not parsed" << std::endl;
    }
}