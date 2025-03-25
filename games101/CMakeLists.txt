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
add_executable(Assignment5 Assignment5/main.cpp Assignment5/Object.hpp Assignment5/Vector.hpp Assignment5/Sphere.hpp Assignment5/global.hpp Assignment5/Triangle.hpp Assignment5/Scene.cpp Assignment5/Scene.hpp Assignment5/Light.hpp Assignment5/Renderer.cpp)
add_executable(Assignment6 Assignment6/main.cpp Assignment6/Object.hpp Assignment6/Vector.cpp Assignment6/Vector.hpp Assignment6/Sphere.hpp Assignment6/global.hpp Assignment6/Triangle.hpp Assignment6/Scene.cpp
        Assignment6/Scene.hpp Assignment6/Light.hpp Assignment6/AreaLight.hpp Assignment6/BVH.cpp Assignment6/BVH.hpp Assignment6/Bounds3.hpp Assignment6/Ray.hpp Assignment6/Material.hpp Assignment6/Intersection.hpp
        Assignment6/Renderer.cpp Assignment6/Renderer.hpp)

# 链接OpenCV库文件
target_link_libraries(Assignment1 ${OpenCV_LIBS})
target_link_libraries(Assignment2 ${OpenCV_LIBS})
target_link_libraries(Assignment3 ${OpenCV_LIBS})
target_link_libraries(Assignment4 ${OpenCV_LIBS})
target_link_libraries(Assignment5 ${OpenCV_LIBS})
target_link_libraries(Assignment6 ${OpenCV_LIBS})
