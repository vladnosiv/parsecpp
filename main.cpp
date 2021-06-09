#include "CalcParser.hpp"

#include <string>
#include <iostream>

int main() {
    auto parser = CalcParser::romanCalc();
    std::string n = "MMMCCCXXI*MMMMMMMMMCXXIII/(II*IV+I)"; // 3321*9123/9 = 3366387
    auto res = parser.eval(n);
    if (res.has_value()) {
        std::cout << res.value().first;
    } else {
        std::cout << "Not parsed.";
    }
}