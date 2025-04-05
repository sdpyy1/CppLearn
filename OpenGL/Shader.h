//
// Created by Administrator on 2025/4/4.
//

#ifndef OPENGL_SHADER_H
#define OPENGL_SHADER_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/fwd.hpp>

class Shader {
public:
    GLuint ID;
    Shader(const char*vertexPath, const char*fragmentPath);

    void use();
    // uniform工具函数
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec4f(const std::string &name, float v0, float v1, float v2, float v3) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

};


#endif //OPENGL_SHADER_H
