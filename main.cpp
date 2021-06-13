#include "CalcParser.hpp"

#include <string>
#include <string_view>
#include <iostream>

int main() {
    auto parser = CalcParser::roman_calc();

    //std::string n = "MMMCCCXXI*MMMMMMMMMCXXIII/(II*IV+(-(-I)))"; // 3321*9123/(2*4+(-(-1))) = 3366387
    std::string str;
    while (std::cin >> str) {
        auto result = parser.parse(str);
        if (result && result.rest().empty()) {
            std::cout << result.value() << '\n';
        } else {
            std::cout << "error: parsing failed\n";
        }
    }
}