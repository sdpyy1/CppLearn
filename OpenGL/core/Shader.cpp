#include "Shader.h"

#include "../utils/checkGlCommand.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "stb_image.h"

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

Shader::Shader(const char* computePath) {
    // 1. 读取计算着色器源码
    std::string computeCode;
    std::ifstream cShaderFile;
    cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        cShaderFile.open(computePath);
        std::stringstream cShaderStream;
        cShaderStream << cShaderFile.rdbuf();
        cShaderFile.close();
        computeCode = cShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << computePath << std::endl;
    }
    const char* cShaderCode = computeCode.c_str();

    // 2. 编译 Compute Shader
    unsigned int compute;
    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cShaderCode, nullptr);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE");

    // 3. 创建程序并链接
    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // 4. 删除着色器对象
    glDeleteShader(compute);
}

void Shader::bind() {
    glUseProgram(ID);
    isUsing = true;
}

void Shader::setBool(const std::string& name, bool value) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string& name, float x, float y) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void Shader::setVec3i(const std::string& name, const glm::ivec3& value) const
{
    if (!isUsing) {
        std::cerr << "Shader未use" << "\n";
        return;
    }
    // 使用glUniform3iv设置整数向量，注意函数后缀是iv
    glUniform3iv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    if (!isUsing){
        std::cerr << "Shader未use" << "\n";
        return;
    }
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::bindTexture(const std::string& name, unsigned int texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);
    setInt(name, slot);
}

void Shader::bindCubeMapTexture(const std::string& name, unsigned int texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    setInt(name, slot);
}
void Shader::bind3DTexture(const std::string& name, unsigned int texture, int slot)
{
    // 激活指定的纹理单元
    glActiveTexture(GL_TEXTURE0 + slot);

    // 绑定3D纹理，使用正确的GL_TEXTURE_3D目标
    glBindTexture(GL_TEXTURE_3D, texture);

    // 将纹理单元索引传递给着色器中的uniform变量
    setInt(name, slot);
}
// 加载 3D 纹理，返回 OpenGL 纹理 ID
GLuint Shader::load3DTextureFromFile(const std::string& filename, int width, int height, int depth)
{
    // 打开二进制文件
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 0;
    }

    // 读取所有数据
    std::vector<unsigned char> data(width * height * depth);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    file.close();

    if (file.gcount() != data.size())
    {
        std::cerr << "File size mismatch!" << std::endl;
        return 0;
    }

    // 创建 3D 纹理
    GLuint tex3D;
    glGenTextures(1, &tex3D);
    glBindTexture(GL_TEXTURE_3D, tex3D);

    // 上传数据到 GPU
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, width, height, depth, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glBindTexture(GL_TEXTURE_3D, 0);

    return tex3D;
}

GLuint Shader::loadTextureFormFile(const std::string& filePath)
{

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << filePath << std::endl;
        stbi_image_free(data);
    }
    return textureID;
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

void Shader::unBind() {
    if(isUsing){
        glUseProgram(0);
        isUsing = false;
    }
}
