cmake_minimum_required(VERSION 3.17)
project(CalcParserTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wpedantic")

add_executable(CalcParserTest Test.cpp CalcParserTest.cpp)
add_test(NAME CalcParserTest COMMAND CalcParserTest)
