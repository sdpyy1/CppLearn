cmake_minimum_required(VERSION 3.20)
project(OpenGLRender)
set(CMAKE_CXX_STANDARD 20)

# 添加头文件检索路径
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third/include)

# 源文件
set(SOURCES
        third/src/glad.c
        main.cpp
        core/VertexArray.cpp
        core/VertexBuffer.cpp
        core/IndexBuffer.cpp
        core/WindowManager.cpp
        utils/shaderUtils.h
)

add_executable(OpenGLRender ${SOURCES})

if(WIN32)
    target_link_libraries(OpenGLRender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libassimp.dll.a)
    target_link_libraries(OpenGLRender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libglfw3.a)
elseif(APPLE)
    target_link_libraries(OpenGLRender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libglfw.3.dylib)
endif()

# 复制lib目录到输出目录（执行目录）
add_custom_command(TARGET OpenGLRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/third/lib
        $<TARGET_FILE_DIR:OpenGLRender>
)

add_custom_command(TARGET OpenGLRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shader
        $<TARGET_FILE_DIR:OpenGLRender>/shader
        COMMENT "Copy entire shader directory to output folder"
)