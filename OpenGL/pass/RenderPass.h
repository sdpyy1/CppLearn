#pragma once

#include <unordered_map>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

// 统一管理资源
struct RenderResource {
    std::unordered_map<std::string, GLuint> textures;
    std::unordered_map<std::string, GLuint> framebuffers;
    std::unordered_map<std::string, glm::mat4> matrices;
    std::unordered_map<std::string, float> scalars;
    // 可继续扩展 uniform buffer, light data, etc.
};
class RenderPass
{
public:
    std::string passName = "Unknown";
    bool isInit = false;
    virtual void init(RenderResource& resource){
        std::cout << "Init {" << passName <<"} done!"<< std::endl;
        isInit = true;
    };

    virtual void render(RenderResource& resource) = 0;
    virtual ~RenderPass() = default;
};



