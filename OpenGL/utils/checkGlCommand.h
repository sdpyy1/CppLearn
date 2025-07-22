//
// Created by Administrator on 2025/4/6.
//

#ifndef OPENGL_MACROS_H
#define OPENGL_MACROS_H
#include <iostream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GL_CALL(x) GLClearError();x;ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static  void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}
static bool  GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ")" << function <<
                  " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

#endif //OPENGL_MACROS_H
