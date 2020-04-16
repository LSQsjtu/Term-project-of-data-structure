cmake_minimum_required(VERSION 3.15)
project(deque)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(data)

add_executable(deque
        data/one/code.cpp
        data/class-bint.hpp
        data/class-integer.hpp
        data/class-matrix.hpp
        deque.hpp
        exceptions.hpp
        utility.hpp)
