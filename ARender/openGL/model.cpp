#pragma once

#include "logger.h"
#include "model.h"

void Model::printModelInfo() {
    // 打印模型基本信息
    Logger::info("===== Model Information =====");

    // 打印状态信息
    QString statusStr;
    switch(_status) {
    case LOADING: statusStr = "LOADING"; break;
    case LOADED: statusStr = "LOADED"; break;
    case ERR: statusStr = "ERROR"; break;
    }
    Logger::info("Status: " + statusStr.toStdString());

    // 打印网格信息
    Logger::info("Mesh count: " + QString::number(_meshes.size()).toStdString());
    for(size_t i = 0; i < _meshes.size(); ++i) {
        Logger::info((QString("  Mesh %1: %2 vertices, %3 indices")
                         .arg(i)
                         .arg(_meshes[i]._vertices.size())
                         .arg(_meshes[i]._indices.size())).toStdString());
    }

    // 打印纹理信息
    Logger::info("Texture count: " + QString::number(_texturesLoaded.size()).toStdString());
    for(const auto& tex : _texturesLoaded) {
        QString typeStr;
        switch(tex._type) {
        case DIFFUSE: typeStr = "DIFFUSE"; break;
        case SPECULAR: typeStr = "SPECULAR"; break;
        }
        Logger::info((QString("  Texture ID: %1, Type: %2, Path: %3")
                         .arg(tex._id)
                         .arg(typeStr)
                         .arg(QString::fromStdString(tex._path))).toStdString());
    }
    Logger::info("=============================");
}

Model::Model(std::string path) {
    // Load model meta info from path
    // TODO: Implement as async task
    loadModel(path);
    printModelInfo();
}

Model::~Model() {
    // TODO: Maybe delete all meshes?
    // 删除所有网格
    _meshes.clear();
    // 可以考虑其他资源清理，如纹理等
    _texturesLoaded.clear();
}

Model::Model(std::vector<Mesh>&& meshes, std::vector<Texture>&& textures, std::string directory, Boundary boundBox) {
    _meshes = std::move(meshes);
    _texturesLoaded = std::move(textures);
    _directory = directory;
    _status = LOADED;
    _boundBox = boundBox;
}

void Model::loadModel(std::string path) {
    Logger::debug("加载模型路径：" + path);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Logger::error("Failed to load model: " + std::string(importer.GetErrorString()));
        _status = ERR;
        return;
    }

    // Convert all '\' to '/'
    std::replace(path.begin(), path.end(), '\\', '/');
    _directory = path.substr(0, path.find_last_of('/'));

    Logger::debug("Model read successfully");
    Logger::debug("Processing model nodes");
    processNode(scene->mRootNode, scene);
    _status = LOADED;
    Logger::debug("Model loaded");
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process all meshes in node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        _meshes.push_back(processMesh(mesh, scene));
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Logger::debug("Processing mesh with " + std::to_string(mesh->mNumVertices) + " vertices");
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // Create placeholder vectors
        glm::vec3 vertexPosition = glm::vec3(0.0f);
        glm::vec3 vertexNormal = glm::vec3(0.0f);
        glm::vec2 vertexTextureCoordinate = glm::vec2(0.0f);
        glm::vec3 vertexTangent = glm::vec3(0.0f);
        glm::vec3 vertexBitangent = glm::vec3(0.0f);

        // Process vertex position
        vertexPosition.x = mesh->mVertices[i].x;
        vertexPosition.y = mesh->mVertices[i].y;
        vertexPosition.z = mesh->mVertices[i].z;

        // Update boundary box
        _boundBox.updateControlPoints(vertexPosition);

        // Process vertex normals
        if (mesh->mNormals) {
            vertexNormal.x = mesh->mNormals[i].x;
            vertexNormal.y = mesh->mNormals[i].y;
            vertexNormal.z = mesh->mNormals[i].z;
        }

        // Process vertex texture coordinates
        if (mesh->mTextureCoords[0]) {
            // Process texture coordinates
            vertexTextureCoordinate.x = mesh->mTextureCoords[0][i].x;
            vertexTextureCoordinate.y = mesh->mTextureCoords[0][i].y;

            //// Process vertex tangents
            //if (mesh->mTangents) {
            //    vertexTangent.x = mesh->mTangents[i].x;
            //    vertexTangent.y = mesh->mTangents[i].y;
            //    vertexTangent.z = mesh->mTangents[i].z;
            //}
            //
            //// Process vertex bitangents
            //if (mesh->mBitangents) {
            //    vertexBitangent.x = mesh->mBitangents[i].x;
            //    vertexBitangent.y = mesh->mBitangents[i].y;
            //    vertexBitangent.z = mesh->mBitangents[i].z;
            //}
        }
        else {
            vertexTextureCoordinate = glm::vec2(0.0f, 0.0f);
        }

        // Create new vertex
        Vertex newVertex = {
            vertexPosition,
            vertexNormal,
            vertexTextureCoordinate,
            //vertexTangent,
            //vertexBitangent
        };

        // Add vertex to vertices
        vertices.push_back(newVertex);
    }
    Logger::debug("Vertices vector memory usage: " + std::to_string(vertices.size() * sizeof(Vertex) / 1024) + " KB");

    Logger::debug("Processing mesh with " + std::to_string(mesh->mNumFaces) + " faces");
    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    Logger::debug("Indices vector memory usage: " + std::to_string(indices.size() * sizeof(unsigned int) / 1024) + " KB");

    Logger::debug("Processing mesh materials");
    // Process material
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // Specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    Logger::debug("Textures vector memory usage: " + std::to_string(textures.size() * sizeof(Texture) / 1024) + " KB");

    Logger::debug("Mesh processed");
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType textureType) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < _texturesLoaded.size(); j++) {
            if (std::strcmp(_texturesLoaded[j].path().data(), str.C_Str()) == 0) {
                textures.push_back(_texturesLoaded[j]);
                skip = true;
                Logger::debug("Texture already loaded, skipped");
                break;
            }
        }
        if (!skip) {
            Texture newTexture(textureType, _directory + '/' + str.C_Str());
            textures.push_back(newTexture);
            _texturesLoaded.push_back(newTexture);
        }
    }
    return textures;
}

void Model::render(const ShaderProgram& shader) const {
    // Test for model status
    if (_status != LOADED) {
        Logger::error("Trying to render unloaded model");
        return;
    }
    // Render the model
    for (unsigned int i = 0; i < _meshes.size(); i++) {
        _meshes[i].render(shader);
    }

    // 渲染完成后解绑纹理
    for(int i = 0;i<_texturesLoaded.size();i++){
        _texturesLoaded[i].unbind();
    }
}

void Model::renderLine(const ShaderProgram& shader)
{
    // Test for model status
    if (_status != LOADED) {
        Logger::error("Trying to render unloaded model");
        return;
    }
    // Render the model
    for (unsigned int i = 0; i < _meshes.size(); i++) {
        _meshes[i].renderLine(shader);
    }
    // 渲染完成后解绑纹理
    for(int i = 0;i<_texturesLoaded.size();i++){
        _texturesLoaded[i].unbind();
    }
}


HitRecord Model::hit(const Ray& ray, const glm::mat4& modelMatrix) const {
    HitRecord record = HitRecord();
    for (unsigned int i = 0; i < _meshes.size(); i++) {
        HitRecord hitRecord = _meshes[i].hit(ray, modelMatrix);
        if (hitRecord.hitted() && hitRecord.t() < record.t()) {
            record = hitRecord;
        }
    }
    return record;
}

Model* Model::copyToCurrentContext() const {
    // Reload all textures
    std::vector<Texture> newTextures;
    for (unsigned int i = 0; i < _texturesLoaded.size(); i++) {
        // Load texture
        Texture newTexture = Texture(_texturesLoaded[i].type(), _directory + '/' + _texturesLoaded[i].path());
        newTextures.push_back(newTexture);
    }

    // Copy all meshes
    std::vector<Mesh> newMeshes;
    for (unsigned int i = 0; i < _meshes.size(); i++) {
        // Copy mesh
        Mesh newMesh = Mesh(_meshes[i].vertices(), _meshes[i].indices(), newTextures);
        newMeshes.push_back(newMesh);
    }

    // Create new model
    Model* newModel = new Model(std::move(newMeshes), std::move(newTextures), _directory, _boundBox);
    return newModel;
}
