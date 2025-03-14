cmake_minimum_required(VERSION 3.25)
project(games101)

set(CMAKE_CXX_STANDARD 17)
include_directories(Assignment0/eigen-3.4.0)
add_executable(games101 Assignment0/main.cpp)
