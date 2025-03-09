# CMake 学习
# 最低版本要求
cmake_minimum_required(VERSION 3.12)
# 名字
project(cmakeLearn)
# 支持的CPP版本
set(CMAKE_CXX_STANDARD 11)

# 整合资源
file(GLOB ASSETS "./asset")
# Copy资源
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

# 设置主程序名称 + 主程序文件  这里后边只能加cpp文件，h文件会自动搜索，不需要手动
add_executable(proName1 hello.cpp)
# 另外一个程序入口
#add_executable(proName2 base/hello1.cpp)
# 别的地方的头文件需要
include_directories(include)

add_executable(funP 函数指针.cpp)