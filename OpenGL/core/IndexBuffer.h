#pragma once

#include <glad/glad.h>
#include <cstddef>

class IndexBuffer
{
public:
    // 构造函数传入 index 数据和数量（单位：个数，不是字节）
    IndexBuffer(const unsigned int* data, size_t count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    size_t GetCount() const { return m_Count; }

private:
    GLuint m_RendererID;
    size_t m_Count;  // 索引数量，用于绘制
};
