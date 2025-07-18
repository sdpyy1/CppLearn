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

using std::string;
using std::vector;

class Model
{
public:
    vector<Texture> textures_loaded; // 防止重复加载纹理
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection = false;
    glm::mat3 modelMatrix{1.0f};

    explicit Model(const string& path, bool gamma = false);

    void draw(Shader& shader);

private:
    void loadModel(const string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const string& typeName);
    unsigned int TextureFromFile(const char* path, const string& directory);
};

#endif // OPENGL_MODEL_H
