cmake_minimum_required(VERSION 3.25)
project(games101)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    # window配置
    set(OpenCV_DIR "D:/context/opencv/opencv/cmakeBuild/install")
    include_directories(thirdParty/eigen3-win)
elseif(APPLE)
    # mac配置
    set(OpenCV_DIR "/opt/homebrew/Cellar/opencv/4.11.0_1")
    include_directories(thirdParty/eigen3-mac)
endif()

find_package(OpenCV REQUIRED)

# 添加OpenCV头文件目录
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(Assignment0 Assignment0/main.cpp)
add_executable(Assignment1 Assignment1/rasterizer.cpp Assignment1/Triangle.cpp Assignment1/main.cpp)
add_executable(Assignment2 Assignment2/rasterizer.cpp Assignment2/Triangle.cpp Assignment2/main.cpp)
add_executable(Assignment3 Assignment3/main.cpp Assignment3/rasterizer.hpp Assignment3/rasterizer.cpp Assignment3/global.hpp Assignment3/Triangle.hpp Assignment3/Triangle.cpp Assignment3/Texture.hpp Assignment3/Texture.cpp Assignment3/Shader.hpp Assignment3/OBJ_Loader.h)
add_executable(Assignment4 Assignment4/main.cpp)

# 链接OpenCV库文件
target_link_libraries(Assignment1 ${OpenCV_LIBS})
target_link_libraries(Assignment2 ${OpenCV_LIBS})
target_link_libraries(Assignment3 ${OpenCV_LIBS})
target_link_libraries(Assignment4 ${OpenCV_LIBS})
