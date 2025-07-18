#include "Model.h"
#include <iostream>
#include <stb_image.h>
#include <cstring>

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
}

void Model::draw(Shader& shader)
{
    for (auto& mesh : meshes)
    {
        shader.setMat4("model", glm::mat4(modelMatrix)); // glm::mat3转mat4，注意
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
                                             aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                             aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices | aiProcess_GlobalScale);

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

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // 打印所有导入的纹理
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        aiMaterial* material = scene->mMaterials[i];

        std::cout << "Material " << i << ":\n";
        for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; ++type) {
            aiTextureType texType = static_cast<aiTextureType>(type);
            unsigned int texCount = material->GetTextureCount(texType);
            for (unsigned int j = 0; j < texCount; ++j) {
                aiString path;
                if (material->GetTexture(texType, j, &path) == AI_SUCCESS) {
                    std::cout << "  Texture Type " << type << " : " << path.C_Str() << "\n";
                }
            }
        }
    }
    vector<Texture> albedoMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_albedo");
    textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());

    vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    vector<Texture> metallicMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metallic");
    textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

    vector<Texture> roughnessMaps =
        loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

    vector<Texture> aoMaps = loadMaterialTextures(material, aiTextureType_LIGHTMAP, "texture_ao");
    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

    vector<Texture> emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emission");
    textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());

    return Mesh(vertices, indices, textures);
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