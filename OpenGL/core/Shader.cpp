#include "Shader.h"

#include "../utils/checkGlCommand.h"

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vStream, fStream;
        vStream << vShaderFile.rdbuf();
        fStream << fShaderFile.rdbuf();
        vertexCode = vStream.str();
        fragmentCode = fStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_READ: " << e.what() << "\n";
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    GL_CALL(vertex = glCreateShader(GL_VERTEX_SHADER));
    GL_CALL(glShaderSource(vertex, 1, &vShaderCode, nullptr));
    GL_CALL(glCompileShader(vertex));
    checkCompileErrors(vertex, "VERTEX");

    GL_CALL(fragment = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CALL(glShaderSource(fragment, 1, &fShaderCode, nullptr));
    GL_CALL(glCompileShader(fragment));
    checkCompileErrors(fragment, "FRAGMENT");

    GL_CALL(ID = glCreateProgram());
    GL_CALL(glAttachShader(ID, vertex));
    GL_CALL(glAttachShader(ID, fragment));
    GL_CALL(glLinkProgram(ID));
    checkCompileErrors(ID, "PROGRAM");

    GL_CALL(glDeleteShader(vertex));
    GL_CALL(glDeleteShader(fragment));
}

Shader::Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath)
{
    std::string vertexCode, geometryCode, fragmentCode;
    std::ifstream vFile, gFile, fFile;
    vFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vFile.open(vertexPath);
        gFile.open(geometryPath);
        fFile.open(fragmentPath);
        std::stringstream vStream, gStream, fStream;
        vStream << vFile.rdbuf();
        gStream << gFile.rdbuf();
        fStream << fFile.rdbuf();
        vertexCode = vStream.str();
        geometryCode = gStream.str();
        fragmentCode = fStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_READ: " << e.what() << "\n";
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, geometry, fragment;
    GL_CALL(vertex = glCreateShader(GL_VERTEX_SHADER));
    GL_CALL(glShaderSource(vertex, 1, &vShaderCode, nullptr));
    GL_CALL(glCompileShader(vertex));
    checkCompileErrors(vertex, "VERTEX");

    GL_CALL(geometry = glCreateShader(GL_GEOMETRY_SHADER));
    GL_CALL(glShaderSource(geometry, 1, &gShaderCode, nullptr));
    GL_CALL(glCompileShader(geometry));
    checkCompileErrors(geometry, "GEOMETRY");

    GL_CALL(fragment = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CALL(glShaderSource(fragment, 1, &fShaderCode, nullptr));
    GL_CALL(glCompileShader(fragment));
    checkCompileErrors(fragment, "FRAGMENT");

    GL_CALL(ID = glCreateProgram());
    GL_CALL(glAttachShader(ID, vertex));
    GL_CALL(glAttachShader(ID, geometry));
    GL_CALL(glAttachShader(ID, fragment));
    GL_CALL(glLinkProgram(ID));
    checkCompileErrors(ID, "PROGRAM");

    GL_CALL(glDeleteShader(vertex));
    GL_CALL(glDeleteShader(geometry));
    GL_CALL(glDeleteShader(fragment));
}

void Shader::use() const
{
    GL_CALL(glUseProgram(ID));
}

void Shader::setBool(const std::string& name, bool value) const
{
    GL_CALL(glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value));
}

void Shader::setInt(const std::string& name, int value) const
{
    GL_CALL(glUniform1i(glGetUniformLocation(ID, name.c_str()), value));
}

void Shader::setFloat(const std::string& name, float value) const
{
    GL_CALL(glUniform1f(glGetUniformLocation(ID, name.c_str()), value));
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    GL_CALL(glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]));
}

void Shader::setVec2(const std::string& name, float x, float y) const
{
    GL_CALL(glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    GL_CALL(glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]));
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    GL_CALL(glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    GL_CALL(glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]));
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    GL_CALL(glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w));
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    GL_CALL(glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]));
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    GL_CALL(glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    GL_CALL(glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]));
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        if (!success)
        {
            GL_CALL(glGetShaderInfoLog(shader, 1024, NULL, infoLog));
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        GL_CALL(glGetProgramiv(shader, GL_LINK_STATUS, &success));
        if (!success)
        {
            GL_CALL(glGetProgramInfoLog(shader, 1024, NULL, infoLog));
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
