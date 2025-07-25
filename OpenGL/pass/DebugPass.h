//
// Created by Administrator on 2025/7/25.
//

#ifndef OPENGLRENDER_DEBUGPASS_H
#define OPENGLRENDER_DEBUGPASS_H


#include "RenderPass.h"
#include "../core/Shader.h"
#include "../helper/Scene.h"

class DebugPass : public RenderPass{
public:
    explicit DebugPass(Scene &scene);
    Shader skyboxShader;
    Shader lightCubeShader;
    Shader outlineShader;
    void init(RenderResource& resource) override;
    void render(RenderResource& resource) override;
    Scene &scene;
};


#endif //OPENGLRENDER_DEBUGPASS_H
