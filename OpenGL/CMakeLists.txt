cmake_minimum_required(VERSION 3.20)
project(OpenGL)
set(CMAKE_CXX_STANDARD 20)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third/include)
add_executable(OpenGLDemo third/src/glad.c main.cpp Shader.cpp)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/third/lib)
if(WIN32)
    # window配置
    target_link_libraries(OpenGLDemo ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/glfw3.dll)
elseif(APPLE)
    # mac配置
    target_link_libraries(OpenGLDemo ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libglfw.3.dylib)
endif()