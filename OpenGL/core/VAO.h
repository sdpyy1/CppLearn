#pragma once
#include <glad/glad.h>

class VAO {
public:
    VAO();
    ~VAO();

    void bind() const;
    void unbind() const;

    void addAttrib(unsigned int index, int size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

private:
    unsigned int ID;
};
