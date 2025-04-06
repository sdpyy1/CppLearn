//
// Created by Administrator on 2025/4/6.
//

#ifndef OPENGL_MACROS_H
#define OPENGL_MACROS_H
#define GL_CALL(x) \
    do { \
        x; \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            std::cerr << "OpenGL error: " << error << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        } \
    } while (0)
#endif //OPENGL_MACROS_H
