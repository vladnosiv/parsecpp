#include "CalcParser.hpp"

#include <string>
#include <iostream>

int main() {
    auto parser = CalcParser::roman_calc();
    std::string n = "MMMCCCXXI*MMMMMMMMMCXXIII/(II*IV+(-(-I))))"; // 3321*9123/(2*4+(-(-1))) = 3366387
    auto res = parser.eval(n);
    if (res) {
        std::cout << res.value();
    } else {
        std::cout << "Not parsed.";
    }
}