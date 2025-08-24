#include "Mesh.h"

#include <iostream>


#include "Model.h"
#include "stb_image.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, PBRMaterial mat)
    : vertices(std::move(vertices)), indices(std::move(indices)), mat(mat)
{
    setupMesh();
}
unsigned int TextureFromFile(const char* path)
{
    string filename = path;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
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
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
void Mesh::loadNewTexture(const std::string& path, const std::string& typeName)
{
    GLuint texID = TextureFromFile(path.c_str());

    if (typeName == "texture_albedo") {
        mat.albedo = texID;
        mat.hasAlbedo = true;
    }
    else if (typeName == "texture_normal") {
        mat.normal = texID;
        mat.hasNormal = true;
    }
    else if (typeName == "texture_metallic") {
        mat.metallic = texID;
        mat.hasMetallic = true;
    }
    else if (typeName == "texture_roughness") {
        mat.roughness = texID;
        mat.hasRoughness = true;
    }
    else if (typeName == "texture_ao") {
        mat.ao = texID;
        mat.hasAO = true;
    }
    else if (typeName == "texture_emission") {
        mat.emission = texID;
        mat.hasEmission = true;
    }
    else {
        // 你可以选择打印警告或者忽略
        std::cerr << "Warning: Unknown texture type: " << typeName << std::endl;
    }
}


void Mesh::draw(Shader& shader)
{
    shader.bindTexture("texture_albedo",mat.albedo,0);
    shader.bindTexture("texture_normal",mat.normal,1);
    shader.bindTexture("texture_metallic",mat.metallic,2);
    shader.bindTexture("texture_roughness",mat.roughness,3);
    shader.bindTexture("texture_ao",mat.ao,4);
    shader.bindTexture("texture_emission",mat.emission,5);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}
