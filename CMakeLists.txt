cmake_minimum_required(VERSION 3.30)
project(pong VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
include_directories(vendor/Zeuron/include)
include_directories(vendor/Zeuron/vendor/fenster)

add_executable(pong src/Pong.cpp)