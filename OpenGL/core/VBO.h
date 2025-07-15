#pragma once
#include <glad/glad.h>
#include <cstddef>
class VBO {
public:
    VBO();
    ~VBO();

    void setData(const void* data, size_t size);
    void bind() const;
    void unbind() const;

private:
    unsigned int ID;
};
