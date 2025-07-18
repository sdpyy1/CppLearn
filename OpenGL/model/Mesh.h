#ifndef OPENGL_MESH_H
#define OPENGL_MESH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <vector>
#include "../core/Shader.h"

using std::vector;
using std::string;

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture
{
    unsigned int id;
    string type;
    string path;
};

class Mesh
{
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    void loadNewTexture(const string& path,const string& typeName);

    void draw(Shader& shader);

private:
    unsigned int VBO, EBO;

    void setupMesh();
};

#endif //OPENGL_MESH_H
