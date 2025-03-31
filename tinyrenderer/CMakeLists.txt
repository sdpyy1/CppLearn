cmake_minimum_required(VERSION 3.12...3.26)
set(CMAKE_CXX_STANDARD 20)

project(tinyrenderer)
add_executable(tinyrenderer main.cpp tgaimage.cpp)

