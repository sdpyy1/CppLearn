cmake_minimum_required(VERSION 3.20)
project(OpenGL)
set(CMAKE_CXX_STANDARD 20)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third/include)
add_executable(OpenGL third/src/glad.c main.cpp)
add_executable(FirstTry first.cpp third/src/glad.c)
LINK_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third/lib)
if(WIN32)
    # window配置
    target_link_libraries(OpenGL ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/glfw3.dll)
elseif(APPLE)
    # mac配置
    target_link_libraries(OpenGL ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libglfw.3.dylib)
endif()