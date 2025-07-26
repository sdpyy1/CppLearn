#pragma once

#include <unordered_map>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <utility>
#include "../core/Shader.h"

// 统一管理资源
struct RenderResource {
    std::unordered_map<std::string, GLuint> textures;
    std::unordered_map<std::string, GLuint> framebuffers;
    std::unordered_map<std::string, glm::mat4> matrices;
    std::unordered_map<std::string, GLuint> VAOs;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
};
class RenderPass
{
public:
    explicit RenderPass(std::string passName):passName(std::move(passName)){};
    std::string passName = "Unknown";
    bool isInit = false;
    virtual void init(RenderResource& resource){
        std::cout << "Init {" << passName <<"} done!"<< std::endl;
        isInit = true;
    };

    virtual void render(RenderResource& resource) = 0;
    virtual ~RenderPass() = default;
};



