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

struct BoneInfo {
    glm::mat4 offsetMatrix;       // 骨骼偏移矩阵（初始绑定姿态的逆矩阵）
    glm::mat4 finalTransformation; // 最终动画计算出的骨骼变换矩阵

    BoneInfo() {
        offsetMatrix = glm::mat4(1.0f);
        finalTransformation = glm::mat4(1.0f);
    }
};

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    int boneIDs[4] = {0};
    float weights[4] = {0.0f};

    void AddBoneData(int boneID, float weight) {
        for (int i = 0; i < 4; i++) {
            if (weights[i] == 0.0f) {
                boneIDs[i] = boneID;
                weights[i] = weight;
                return;
            }
        }
        // 超过4个骨骼影响就忽略（或处理溢出）
    }
};
struct PBRMaterial {
    GLuint albedo     = 0;
    GLuint normal     = 0;
    GLuint metallic   = 0;
    GLuint roughness  = 0;
    GLuint ao         = 0;
    GLuint emission   = 0;

    bool hasAlbedo    = false;
    bool hasNormal    = false;
    bool hasMetallic  = false;
    bool hasRoughness = false;
    bool hasAO        = false;
    bool hasEmission  = false;
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
    PBRMaterial mat;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, PBRMaterial mat);
    void loadNewTexture(const string& path,const string& typeName);

    void draw(Shader& shader);

private:
    unsigned int VBO, EBO;

    void setupMesh();
};

#endif //OPENGL_MESH_H
