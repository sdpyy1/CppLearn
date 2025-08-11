#pragma once

#include <glad/glad.h>
#include <iostream>
#include "RenderPass.h"
#include "../core/Shader.h"
#include "../helper/Scene.h"
#include "../utils/checkGlCommand.h"

class GeometryPass final : public RenderPass {
public:
    explicit GeometryPass(Scene &scene);

    void init(RenderResource &resource) override;

    void render(RenderResource &resource) override;

private:
    GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gMaterial = 0, gDepth = 0, gEmission = 0;
    Shader shader;
    Scene &scene;
    GLuint gBuffer = 0;
};
