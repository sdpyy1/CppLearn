#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &ID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &ID);
}

void VAO::bind() const
{
    glBindVertexArray(ID);
}

void VAO::unbind() const
{
    glBindVertexArray(0);
}

void VAO::addAttrib(unsigned int index, int size, GLenum type, GLboolean normalized, GLsizei stride,
                    const void* pointer)
{
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}
