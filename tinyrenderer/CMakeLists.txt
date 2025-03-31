cmake_minimum_required(VERSION 3.12...3.26)
set(CMAKE_CXX_STANDARD 20)
include_directories(../games101/thirdParty/eigen3-mac)
project(tinyrenderer)
add_executable(tinyrenderer main.cpp thirdParty/tgaimage.cpp model.cpp Triangle.h Triangle.cpp)

