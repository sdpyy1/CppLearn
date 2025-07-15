#include "VBO.h"

VBO::VBO() {
    glGenBuffers(1, &ID);
}

VBO::~VBO() {
    glDeleteBuffers(1, &ID);
}

void VBO::setData(const void* data, size_t size) {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void VBO::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
