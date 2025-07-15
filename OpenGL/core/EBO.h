#pragma once
#include <glad/glad.h>
#include <cstddef>
class EBO {
public:
    EBO();
    ~EBO();

    void setData(const void* data, size_t size) const;
    void bind() const;
    void unbind() const;

private:
    unsigned int ID{};
};
