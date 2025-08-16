#ifndef OPENGL_MACROS_H
#define OPENGL_MACROS_H
#include <iostream>
#include <csignal>

// 跨平台 DEBUG_BREAK 宏
#if defined(_WIN32) || defined(_WIN64)
    #define DEBUG_BREAK() __debugbreak()
#elif defined(__APPLE__) || defined(__linux__)
    #define DEBUG_BREAK() raise(SIGTRAP)
#else
    #define DEBUG_BREAK() ((void)0)
#endif

// ASSERT 宏
#define ASSERT(x) if (!(x)) DEBUG_BREAK()

// GL_CALL 宏
#define GL_CALL(x) GLClearError(); x; ASSERT(GLLogCall(#x, __FILE__, __LINE__))

// 清理 OpenGL 错误
static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

// 打印 OpenGL 错误
static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ") " << function <<
                  " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

#endif //OPENGL_MACROS_H
