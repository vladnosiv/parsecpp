#include "CalcParser.hpp"

#include <string>
#include <iostream>

int main() {
    auto parser = CalcParser::roman_calc();
    std::string n = "MMMCCCXXI*MMMMMMMMMCXXIII/(II*IV+(-(-I))))"; // 3321*9123/(2*4+(-(-1))) = 3366387
    auto res = parser.eval(n);
    if (res.has_value()) {
        std::cout << res.value().first;
    } else {
        std::cout << "Not parsed.";
    }
}