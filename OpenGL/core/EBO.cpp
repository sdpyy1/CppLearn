#include "EBO.h"
EBO::EBO() {
    glGenBuffers(1, &ID);
}

EBO::~EBO() {
    glDeleteBuffers(1, &ID);
}

void EBO::setData(const void* data, size_t size) const {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void EBO::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
