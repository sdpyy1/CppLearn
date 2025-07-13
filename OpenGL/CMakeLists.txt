cmake_minimum_required(VERSION 3.20)
project(OpenGL)
set(CMAKE_CXX_STANDARD 20)

INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third/include)
# 定义复制文件夹的自定义命令
add_executable(OpenGLDemo third/src/glad.c main.cpp core/camera.h core/Shader.h
        macros.h
        model/mesh.h
        model/model.h
        core/VertexArray.cpp
        core/VertexArray.h
        core/VertexBuffer.cpp
        core/VertexBuffer.h
        core/IndexBuffer.cpp
        core/IndexBuffer.h
        core/WindowManager.cpp
        core/WindowManager.h)
add_custom_command(
        TARGET OpenGLDemo  # 替换为你的目标（如可执行文件）
        POST_BUILD          # 在构建完成后执行
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shader
        ${CMAKE_CURRENT_BINARY_DIR}/shader
        COMMENT "Copying assets and shaders to build directory..."
)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/third/lib)
if(WIN32)
    target_link_libraries(OpenGLDemo ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libassimp.dll.a)
    # window配置
    target_link_libraries(OpenGLDemo ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/glfw3.dll )
elseif(APPLE)
    # mac配置
    target_link_libraries(OpenGLDemo ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libglfw.3.dylib)
endif()

# 2. 构建时复制资源文件

