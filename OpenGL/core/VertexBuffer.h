#pragma once

#include <glad/glad.h>
#include <cstddef>  // 或者 #include <stddef.h>

class VertexBuffer
{
public:
    VertexBuffer(const void* data, size_t size, GLenum usage = GL_STATIC_DRAW);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

private:
    GLuint m_RendererID;
};
