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

    // 如果 Y 方向最高，说明模型立着，需要旋转到卧倒状态（z轴为高）
    if (size.y > size.x && size.y > size.z)
    {
        rotation.x = 90.0f;  // 沿 X 轴旋转 90 度
    }
    else
    {
        rotation = glm::vec3(0.0f); // 不旋转
    }
}


glm::mat4 Model::getModelMatrix() const {
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), translation);
    mat = mat * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0));
    mat = mat * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0));
    mat = mat * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));
    mat = mat * glm::scale(glm::mat4(1.0f), scale);
    return mat;
}

Model Model::createPlane(float size) {
    Model planeModel;

    float halfSize = size / 2.0f;

    // 4 顶点数据
    Vertex v0 = { {-halfSize, 0.0f, -halfSize}, {0,1,0}, {0,0} };
    Vertex v1 = { { halfSize, 0.0f, -halfSize}, {0,1,0}, {1,0} };
    Vertex v2 = { { halfSize, 0.0f,  halfSize}, {0,1,0}, {1,1} };
    Vertex v3 = { {-halfSize, 0.0f,  halfSize}, {0,1,0}, {0,1} };

    std::vector<Vertex> vertices = {v0, v1, v2, v3};
    std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};
    float uvTiling = 10.0f;
    for (auto& v : vertices) {
        v.TexCoords *= uvTiling;
    }
    // 计算两个三角形的切线副切线
    auto computeTBN = [](Vertex& v0, Vertex& v1, Vertex& v2) {
        glm::vec3 edge1 = v1.Position - v0.Position;
        glm::vec3 edge2 = v2.Position - v0.Position;
        glm::vec2 deltaUV1 = v1.TexCoords - v0.TexCoords;
        glm::vec2 deltaUV2 = v2.TexCoords - v0.TexCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent, bitangent;

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);

        v0.Tangent = v1.Tangent = v2.Tangent = tangent;
        v0.Bitangent = v1.Bitangent = v2.Bitangent = bitangent;
    };

    // 对两个三角形调用计算
    computeTBN(vertices[0], vertices[1], vertices[2]);
    computeTBN(vertices[0], vertices[2], vertices[3]);

    // 创建材质
    PBRMaterial mat;
    mat.metallic  = defaultMetallic;
    mat.emission  = defaultBlack;
    Mesh planeMesh(vertices, indices, mat);
    planeModel.meshes.push_back(planeMesh);

    Mesh &mesh = planeModel.meshes[0];
    mesh.loadNewTexture("assets/floor/albedo.jpg","texture_albedo");
    mesh.loadNewTexture("assets/floor/AO.jpg","texture_ao");
    mesh.loadNewTexture("assets/floor/normal.jpg","texture_normal");
    mesh.loadNewTexture("assets/floor/roughness.jpg","texture_roughness");

    planeModel.translation = glm::vec3(0.0f, -2.f, 0.0f);
    planeModel.directory = "floor";
    return planeModel;
}
Model Model::createArrow(float shaftLength, float shaftRadius, float headLength, float headRadius) {
    Model arrowModel;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // 箭杆 (长方体近似，中心在原点，沿 +Z 轴方向延伸)
    // 四边形的 8 个顶点：
    // 先在 X,Y 平面画圆柱底面近似为矩形，Z轴上两端
    // 这里简化成方柱（可以用圆柱细分）

    // 箭杆顶点(8个顶点，组成两个面)：
    float halfRadius = shaftRadius;
    float shaftEndZ = shaftLength;

    // 底面4个顶点 (z=0)
    vertices.push_back({{-halfRadius, -halfRadius, 0.0f}, {0,0,-1}, {0,0}});
    vertices.push_back({{ halfRadius, -halfRadius, 0.0f}, {0,0,-1}, {1,0}});
    vertices.push_back({{ halfRadius,  halfRadius, 0.0f}, {0,0,-1}, {1,1}});
    vertices.push_back({{-halfRadius,  halfRadius, 0.0f}, {0,0,-1}, {0,1}});

    // 顶面4个顶点 (z=shaftEndZ)
    vertices.push_back({{-halfRadius, -halfRadius, shaftEndZ}, {0,0,1}, {0,0}});
    vertices.push_back({{ halfRadius, -halfRadius, shaftEndZ}, {0,0,1}, {1,0}});
    vertices.push_back({{ halfRadius,  halfRadius, shaftEndZ}, {0,0,1}, {1,1}});
    vertices.push_back({{-halfRadius,  halfRadius, shaftEndZ}, {0,0,1}, {0,1}});

    // 箭杆索引 (组成长方体6个面，每面两个三角形)
    unsigned int baseIndex = 0;
    // 底面
    indices.insert(indices.end(), {baseIndex, baseIndex+1, baseIndex+2, baseIndex, baseIndex+2, baseIndex+3});
    // 顶面
    indices.insert(indices.end(), {baseIndex+4, baseIndex+5, baseIndex+6, baseIndex+4, baseIndex+6, baseIndex+7});
    // 侧面（4个面，每面两个三角形）
    // 0->1->5->4 面
    indices.insert(indices.end(), {0,1,5, 0,5,4});
    // 1->2->6->5 面
    indices.insert(indices.end(), {1,2,6, 1,6,5});
    // 2->3->7->6 面
    indices.insert(indices.end(), {2,3,7, 2,7,6});
    // 3->0->4->7 面
    indices.insert(indices.end(), {3,0,4, 3,4,7});

    // 现在箭头锥体 (4个三角形顶点 + 底面圆)
    // 箭头底面中心点在 shaftEndZ 位置，箭头顶点在 shaftEndZ + headLength 位置

    glm::vec3 headTip(0, 0, shaftEndZ + headLength);

    // 底面4顶点（围绕Z轴，围绕箭杆顶面中心旋转）用正方形近似底面（也可以细分圆形）
    baseIndex = (unsigned int)vertices.size();
    vertices.push_back({{-headRadius, -headRadius, shaftEndZ}, {0,0,-1}, {0,0}});
    vertices.push_back({{ headRadius, -headRadius, shaftEndZ}, {0,0,-1}, {1,0}});
    vertices.push_back({{ headRadius,  headRadius, shaftEndZ}, {0,0,-1}, {1,1}});
    vertices.push_back({{-headRadius,  headRadius, shaftEndZ}, {0,0,-1}, {0,1}});

    // 箭头顶点
    vertices.push_back({headTip, glm::normalize(headTip - glm::vec3(0,0,shaftEndZ)), {0.5f,0.5f}});

    unsigned int tipIndex = (unsigned int)vertices.size() - 1;

    // 箭头底面
    indices.insert(indices.end(), {
            baseIndex, baseIndex+1, baseIndex+2,
            baseIndex, baseIndex+2, baseIndex+3
    });

    // 箭头侧面4个三角形，连接底面四个顶点和箭头顶点
    for (int i = 0; i < 4; ++i) {
        unsigned int next = baseIndex + (i+1) % 4;
        indices.insert(indices.end(), {
                baseIndex + i, next, tipIndex
        });
    }

    // 计算切线和副切线（简单版，法线暂时给固定值）

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i+1]];
        Vertex& v2 = vertices[indices[i+2]];

        glm::vec3 edge1 = v1.Position - v0.Position;
        glm::vec3 edge2 = v2.Position - v0.Position;
        glm::vec2 deltaUV1 = v1.TexCoords - v0.TexCoords;
        glm::vec2 deltaUV2 = v2.TexCoords - v0.TexCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent, bitangent;

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);

        v0.Tangent = tangent; v1.Tangent = tangent; v2.Tangent = tangent;
        v0.Bitangent = bitangent; v1.Bitangent = bitangent; v2.Bitangent = bitangent;
    }

    // 创建材质
    PBRMaterial mat;
    mat.metallic  = defaultMetallic;
    mat.emission  = defaultBlack;
    mat.roughness  = defaultRoughness;
    mat.albedo    = defaultAlbedo;
    mat.ao = defaultAO;
    mat.normal = defaultNormal;

    Mesh arrowMesh(vertices, indices, mat);
    arrowModel.meshes.push_back(arrowMesh);

    arrowModel.directory = "arrow";

    return arrowModel;
}
