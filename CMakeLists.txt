cmake_minimum_required(VERSION 3.30)
project(pong VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
include_directories(vendor/Zeuron/include)
add_subdirectory(vendor/Zeuron)
include_directories(vendor/Zeuron/vendor/AbstractNexus/include)
include_directories(vendor/Zeuron/vendor/ByteStream/include)

add_executable(pong src/Pong.cpp)

target_link_libraries(pong zeuron)