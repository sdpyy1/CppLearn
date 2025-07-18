cmake_minimum_required(VERSION 3.20)
project(OpenGLRender)
set(CMAKE_CXX_STANDARD 20)

# 添加头文件检索路径
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third/include)

# 源文件
file(GLOB CORE_SOURCES "core/*.cpp")
file(GLOB PASS_SOURCES "pass/*.cpp")
file(GLOB MODEL_SOURCES "model/*.cpp")
file(GLOB HELPER_SOURCES "helper/*.cpp")
file(GLOB UTILS_SOURCES "utils/*.cpp")
file(GLOB PRCOMPUTE_SOURCES "precompute/*.cpp")

set(SOURCES
        third/src/glad.c
        main.cpp
        ${CORE_SOURCES}
        ${PASS_SOURCES}
        ${MODEL_SOURCES}
        ${HELPER_SOURCES}
        ${UTILS_SOURCES}
        ${PRCOMPUTE_SOURCES}
)


add_executable(OpenGLRender ${SOURCES})

if (WIN32)
    target_link_libraries(OpenGLRender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libassimp.dll.a)
    target_link_libraries(OpenGLRender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libglfw3.a)
elseif (APPLE)
    find_package(assimp REQUIRED)
    target_link_libraries(OpenGLRender assimp::assimp)
    target_link_libraries(OpenGLRender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libglfw.3.dylib)
endif ()

# 复制lib目录到输出目录（执行目录）
add_custom_command(TARGET OpenGLRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/third/lib
        $<TARGET_FILE_DIR:OpenGLRender>
)
# Copy shader directory
add_custom_command(TARGET OpenGLRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:OpenGLRender>/shader
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shader
        $<TARGET_FILE_DIR:OpenGLRender>/shader
        COMMENT "Clean and copy shader directory to output folder"
)
add_custom_command(TARGET OpenGLRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shader
        $<TARGET_FILE_DIR:OpenGLRender>/shader
        COMMENT "Copy entire shader directory to output folder"
)

add_custom_command(TARGET OpenGLRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:OpenGLRender>/assets
        COMMENT "Copy entire shader directory to output folder"
)