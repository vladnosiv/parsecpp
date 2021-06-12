cmake_minimum_required(VERSION 3.17)
project(vk_core_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wpedantic")

add_executable(vk_core_test Parsec/Parsec.hpp main.cpp)
