#pragma once
#include <glad/glad.h>
#include <vector>
#include "../core/Shader.h"
#include "RenderPass.h"
#include "../helper/Scene.h"

class LightingPass : public RenderPass {
public:
    explicit LightingPass(Scene &scene);

    void init(RenderResource& resource) override;
    void render(RenderResource& resource) override;
    Shader lightingShader;
private:
    void initQuad();
    GLuint quadVAO = 0;
    Scene &scene;
};
