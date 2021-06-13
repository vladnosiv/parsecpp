#include "CalcParser.hpp"

#include <string>
#include <string_view>
#include <iostream>

int main() {
    auto parser = CalcParser::roman_calc();

    //std::string n = "MMMCCCXXI*MMMMMMMMMCXXIII/(II*IV+(-(-I)))"; // 3321*9123/(2*4+(-(-1))) = 3366387
    std::string str;
    while (std::cin >> str) {
        try {
            auto result = parser.parse(str);
            if (result && result.rest().empty()) {
                CalcParser::print_arabic_numeral_to_roman(result.value());
            } else if (result) {
                std::cout << "error: Parsing failed. Part from position " << str.size() - result.rest().size() + 1 << " not parsed." << '\n';
            } else {
                std::cout << "error: Parsing failed. Message: " << result.get_message() << '\n';
            }
        } catch (const std::overflow_error&) {
            std::cout << "error: Overflow int64 error.\n";
        }
    }
}