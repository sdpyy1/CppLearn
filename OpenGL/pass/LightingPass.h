#pragma once
#include <glad/glad.h>
#include <vector>
#include "../core/Shader.h"
#include "RenderPass.h"
#include "../helper/Scene.h"

class LightingPass : public RenderPass {
public:
    explicit LightingPass(Scene &scene);

    void init(RenderResource &resource) override;

    void render(RenderResource &resource) override;

    Shader lightingShader;

private:
    GLuint lightFBO = 0;
    GLuint lightTexture = 0;
    Scene &scene;
};
