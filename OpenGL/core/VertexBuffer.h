#pragma once
#include <glad/glad.h>

class VertexBuffer {
public:
    VertexBuffer();
    ~VertexBuffer();

    void SetData(const void* data, size_t size);
    void Bind() const;
    void Unbind() const;

private:
    unsigned int ID;
};
