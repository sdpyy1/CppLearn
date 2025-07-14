#pragma once
#include <glad/glad.h>

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void AddVertexAttrib(unsigned int index, int size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

private:
    unsigned int ID;
};
