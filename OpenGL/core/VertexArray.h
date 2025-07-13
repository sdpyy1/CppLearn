#pragma once

#include <glad/glad.h>
#include <vector>

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    // 绑定一个 VBO 到当前 VAO，并设置顶点属性
    // index: attribute location
    // size: 元素数量，如 vec3 → 3
    // type: 数据类型，如 GL_FLOAT
    // normalized: 是否归一化
    // stride: 每个顶点总大小（字节）
    // offset: 当前属性在顶点数据中的偏移
    void AddVertexAttribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);

private:
    GLuint m_RendererID;
};