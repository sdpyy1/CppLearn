cmake_minimum_required(VERSION 3.30)
project(OpenGL)

set(CMAKE_CXX_STANDARD 20)
# 假设 GLFW 头文件在源目录的 thirdParty 中
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty)
# 拷贝头文件到构建目录
file(COPY ${INCLUDE_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# 头文件搜索路径
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/thirdParty/include)
# 配置链接目录
link_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib)
# 可执行文件
add_executable(OpenGL main.cpp)
# 会到上边配置的链接目录去找xx.lib
target_link_libraries(OpenGL glfw3.lib)