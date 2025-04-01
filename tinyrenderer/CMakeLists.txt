cmake_minimum_required(VERSION 3.12...3.26)
set(CMAKE_CXX_STANDARD 20)

project(tinyrenderer)
if (WIN32)
    include_directories(../games101/thirdParty/eigen3-win)
endif ()
if (APPLE)
    include_directories(../games101/thirdParty/eigen3-mac)
endif ()


add_executable(tinyrenderer main.cpp thirdParty/tgaimage.cpp model.cpp Triangle.cpp thirdParty/OBJ_Loader.h Texture.h)

