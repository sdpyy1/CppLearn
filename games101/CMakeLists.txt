cmake_minimum_required(VERSION 3.25)
project(games101)

set(CMAKE_CXX_STANDARD 17)
include_directories(.)
# 设置OpenCV的路径
set(OpenCV_DIR "/opt/homebrew/Cellar/opencv/4.11.0_1") # 根据你的OpenCV版本和系统调整路径
# CMake 会在特定的路径里查找 OpenCV 库的配置文件。如果找到了，就会设置一些变量，这样后续就能使用这些变量来链接和包含 OpenCV 库 ${OpenCV_INCLUDE_DIRS} ${OpenCV_LIBS}
find_package(OpenCV REQUIRED)
# 添加OpenCV头文件目录
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(Assignment0 Assignment0/main.cpp)
add_executable(Assignment1 Assignment1/rasterizer.cpp Assignment1/rasterizer.cpp Assignment1/Triangle.cpp Assignment1/main.cpp)
add_executable(Assignment2 Assignment2/rasterizer.cpp Assignment2/rasterizer.cpp Assignment2/Triangle.cpp Assignment2/main.cpp)
# 链接OpenCV库文件
target_link_libraries(Assignment1 ${OpenCV_LIBS})
target_link_libraries(Assignment2 ${OpenCV_LIBS})
