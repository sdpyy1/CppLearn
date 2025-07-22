#include "Model.h"
#include "../utils/checkGlCommand.h"
#include <iostream>
#include <stb_image.h>
#include <cstring>
#include <glm/ext/matrix_transform.hpp>

GLuint Model::defaultAlbedo    = 0;
GLuint Model::defaultNormal    = 0;
GLuint Model::defaultMetallic  = 0;
GLuint Model::defaultRoughness = 0;
GLuint Model::defaultAO        = 0;
GLuint Model::defaultBlack     = 0;
glm::mat4 AssimpToGlm(const aiMatrix4x4& from)
{
    glm::mat4 to;

    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;

    return to;
}
GLuint createNewTextureWithSwizzledChannels(GLuint srcTexID) {
    glBindTexture(GL_TEXTURE_2D, srcTexID);
    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    std::vector<glm::vec3> pixels(width * height);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixels.data());

    for (auto& px : pixels) {
        // 将 G 通道复制到 R 通道
        px.r = px.g;
    }

    GLuint newTex;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_2D, newTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, pixels.data());

    // 设定过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return newTex;
}
Model::Model(const string& path, bool gamma)
    : gammaCorrection(gamma)
{
    loadModel(path);


    std::cout << path << ":{load success}"
        << "\tmeshes:{" << meshes.size() << "}"
        << "\ttexture:{" << textures_loaded.size() << "}" << std::endl;
    for (auto& texture : textures_loaded)
    {
        std::cout << "\ttexture:{" << texture.type << "}" << std::endl;
    }
    calculateOrientationFix();

}

void Model::draw(Shader& shader)
{
    for (auto& mesh : meshes)
    {
        mesh.draw(shader);
    }
}

unsigned int Model::TextureFromFile(const char* path, const string& directory)
{
    string filename = directory + '/' + string(path);

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
void Model::loadModel(const string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
                                             aiProcess_Triangulate  // 全部转化为三角形
                                             | aiProcess_FlipUVs // y坐标反转
                                             |aiProcess_CalcTangentSpace  // 计算切线副切线
                                             | aiProcess_PreTransformVertices // 适用于静态模型  开启后无骨骼信息
                                             | aiProcess_GlobalScale // FBX文件大小单位不一致，缩小模型
                                             );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    // 封装Mesh的顶点、UV坐标、index、切线、副切线
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // 在processMesh末尾，遍历mBones：
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiBone *bone = mesh->mBones[i];
        std::string boneName(bone->mName.C_Str());

        int boneIndex = 0;
        if (boneMapping.find(boneName) == boneMapping.end()) {
            boneIndex = numBones;
            numBones++;
            boneMapping[boneName] = boneIndex;

            BoneInfo bi;
            bi.offsetMatrix = AssimpToGlm(bone->mOffsetMatrix);
            boneInfo.push_back(bi);
        } else {
            boneIndex = boneMapping[boneName];
        }

        // 给顶点分配骨骼权重
        for (unsigned int j = 0; j < bone->mNumWeights; j++) {
            unsigned int vertexID = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            vertices[vertexID].AddBoneData(boneIndex, weight);
        }
    }

    // 开始处理材质
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // 打印所有导入的纹理
//    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
//        aiMaterial* material = scene->mMaterials[i];
//
//        std::cout << "Material " << i << ":\n";
//        for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; ++type) {
//            aiTextureType texType = static_cast<aiTextureType>(type);
//            unsigned int texCount = material->GetTextureCount(texType);
//            for (unsigned int j = 0; j < texCount; ++j) {
//                aiString path;
//                if (material->GetTexture(texType, j, &path) == AI_SUCCESS) {
//                    std::cout << "  Texture Type " << type << " : " << path.C_Str() << "\n";
//                }
//            }
//        }
//    }

    PBRMaterial mat;
    // 定义lambda
    auto loadSingleTexture = [&](aiTextureType type, const std::string& uniformName, GLuint& outID, bool& hasFlag) {
        vector<Texture> loaded = loadMaterialTextures(material, type, uniformName);
        if (!loaded.empty()){
            outID = loaded[0].id;
            hasFlag = true;
        }
    };
    // 尝试从 Assimp 加载各种贴图
    loadSingleTexture(aiTextureType_BASE_COLOR, "texture_albedo", mat.albedo, mat.hasAlbedo);
    loadSingleTexture(aiTextureType_NORMALS, "texture_normal", mat.normal, mat.hasNormal);
    loadSingleTexture(aiTextureType_METALNESS, "texture_metallic", mat.metallic, mat.hasMetallic);
    loadSingleTexture(aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness", mat.roughness, mat.hasRoughness);
    loadSingleTexture(aiTextureType_LIGHTMAP, "texture_ao", mat.ao, mat.hasAO);
    loadSingleTexture(aiTextureType_EMISSIVE, "texture_emission", mat.emission, mat.hasEmission);



    // 若缺失，使用 Scene 中默认贴图
    if (!mat.hasAlbedo)    mat.albedo    = defaultAlbedo;
    if (!mat.hasNormal)    mat.normal    = defaultNormal;
    if (!mat.hasMetallic)  mat.metallic  = defaultMetallic;
    if (!mat.hasRoughness) mat.roughness = defaultRoughness;
    if (!mat.hasAO)        mat.ao        = defaultAO;
    if (!mat.hasEmission)  mat.emission  = defaultBlack;

    // 对金属度和粗糙度在同一张贴图的情况进行处理
    if (mat.roughness == mat.metallic) {
        GLuint newTexId = createNewTextureWithSwizzledChannels(mat.roughness);
        // 替换 roughness 贴图为新生成的
        mat.roughness = newTexId;
    }
    return Mesh(vertices, indices, mat);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const string& typeName)
{
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (auto& item : textures_loaded)
        {
            if (std::strcmp(item.path.data(), str.C_Str()) == 0)
            {
                Texture texture;
                texture.id = item.id;
                texture.path = str.C_Str();
                texture.type = typeName;
                textures.push_back(texture);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

void Model::calculateOrientationFix() {
    glm::vec3 minPos(FLT_MAX);
    glm::vec3 maxPos(-FLT_MAX);

    for (const auto& mesh : meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            minPos = glm::min(minPos, vertex.Position);
            maxPos = glm::max(maxPos, vertex.Position);
        }
    }

    glm::vec3 size = maxPos - minPos;

    // 判断模型高度在哪个轴方向上最大
    if (size.y > size.x && size.y > size.z)
    {
        modelMatrix = glm::rotate(glm::mat4(1.0f),
                                            glm::radians(90.0f),
                                            glm::vec3(1.0f, 0.0f, 0.0f));
    }else{
        modelMatrix = glm::mat4(1.0f);
    }
}
