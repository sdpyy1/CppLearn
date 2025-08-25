#ifndef OPENGL_MODEL_H
#define OPENGL_MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <vector>
#include "../core/Shader.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>

using std::string;
using std::vector;

class Model
{
public:
    vector<Texture> textures_loaded; // 防止重复加载纹理
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection = false;
    glm::vec3 translation = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    std::map<std::string, int> boneMapping; // boneName -> index
    std::vector<BoneInfo> boneInfo;
    int numBones = 0;
    explicit Model(const string& path, bool gamma = false);
    glm::mat4 getModelMatrix() const;

    void draw(Shader& shader);
    static GLuint defaultAlbedo;
    static GLuint defaultNormal;
    static GLuint defaultMetallicZero;
    static GLuint defaultMetallicOne;

    static GLuint defaultRoughnessMid;
    static GLuint defaultRoughnessZero;
    static GLuint defaultAO;
    static GLuint defaultBlack;

    static Model createPlane(float size = 100.0f);
    static Model createArrow(float shaftLength = 1.0f, float shaftRadius = 0.05f, float headLength = 0.2f, float headRadius = 0.1f);
    static Model createCube(float size = 1.f);
    static unsigned int TextureFromFile(const char* path, const string& directory);


private:
    Model()= default;
    void loadModel(const string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const string& typeName);


    void calculateOrientationFix();

    void ReadNodeHierarchy(float animationTime, const aiNode *node, const glm::mat4 &parentTransform);

};

#endif // OPENGL_MODEL_H
