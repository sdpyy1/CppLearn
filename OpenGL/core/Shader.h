#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../utils/macros.h"
class Shader
{
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        GL_CALL(vertex = glCreateShader(GL_VERTEX_SHADER));
        GL_CALL(glShaderSource(vertex, 1, &vShaderCode, NULL));
        GL_CALL(glCompileShader(vertex));
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        GL_CALL(fragment = glCreateShader(GL_FRAGMENT_SHADER));
        GL_CALL(glShaderSource(fragment, 1, &fShaderCode, NULL));
        GL_CALL(glCompileShader(fragment));
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        GL_CALL(ID = glCreateProgram());
        GL_CALL(glAttachShader(ID, vertex));
        GL_CALL(glAttachShader(ID, fragment));
        GL_CALL(glLinkProgram(ID));
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        GL_CALL(glDeleteShader(vertex));
        GL_CALL(glDeleteShader(fragment));

    }
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* geometryPath ,const char* fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            gShaderFile.open(geometryPath);
            std::stringstream vShaderStream, fShaderStream, gShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            gShaderStream << gShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            gShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            geometryCode = gShaderStream.str();
        }catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        const char * gShaderCode = geometryCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment, geometry;
        // vertex shader
        GL_CALL(vertex = glCreateShader(GL_VERTEX_SHADER));
        GL_CALL(glShaderSource(vertex, 1, &vShaderCode, NULL));
        GL_CALL(glCompileShader(vertex));
        checkCompileErrors(vertex, "VERTEX");
        // 几何着色器
        GL_CALL(geometry = glCreateShader(GL_GEOMETRY_SHADER));
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
        // fragment Shader
        GL_CALL(fragment = glCreateShader(GL_FRAGMENT_SHADER));
        GL_CALL(glShaderSource(fragment, 1, &fShaderCode, NULL));
        GL_CALL(glCompileShader(fragment));
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        GL_CALL(ID = glCreateProgram());
        GL_CALL(glAttachShader(ID, vertex));
        GL_CALL(glAttachShader(ID, fragment));
        GL_CALL(glAttachShader(ID, geometry));
        GL_CALL(glLinkProgram(ID));
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        GL_CALL(glDeleteShader(vertex));
        GL_CALL(glDeleteShader(fragment));
        GL_CALL(glDeleteShader(geometry));
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const
    {
        GL_CALL(glUseProgram(ID));
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {
        GL_CALL(glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value));
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    {
        GL_CALL(glUniform1i(glGetUniformLocation(ID, name.c_str()), value));
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    {
        GL_CALL(glUniform1f(glGetUniformLocation(ID, name.c_str()), value));
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const
    {
        GL_CALL(glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]));
    }
    void setVec2(const std::string &name, float x, float y) const
    {
        GL_CALL(glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y));
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    {
        GL_CALL(glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]));
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    {
        GL_CALL(glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z));
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    {
        GL_CALL(glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]));
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const
    {
        GL_CALL(glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w));
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        GL_CALL(glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]));
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        GL_CALL(glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]));
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        GL_CALL(glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]));
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
            if (!success)
            {
                GL_CALL(glGetShaderInfoLog(shader, 1024, NULL, infoLog));
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            GL_CALL(glGetProgramiv(shader, GL_LINK_STATUS, &success));
            if (!success)
            {
                GL_CALL(glGetProgramInfoLog(shader, 1024, NULL, infoLog));
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif