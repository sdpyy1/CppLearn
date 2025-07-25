#pragma once

#include <glad/glad.h>
#include <iostream>
#include "RenderPass.h"
#include "../core/Shader.h"
#include "../helper/Scene.h"
#include "../utils/checkGlCommand.h"

class GeometryPass final : public RenderPass
{
private:
    Shader shader;
    Shader debugShader;
    GLuint debugVAO = 0;
    Scene& scene;
    int width, height;
    GLuint gBuffer = 0;

public:
    GLuint gPosition = 0, gNormal = 0, gAlbedo = 0, gMaterial = 0, gDepth = 0,gEmission = 0;

    explicit GeometryPass(Scene& scene);


    void init(RenderResource& resource) override;
    void render(RenderResource& resource) override;

    void debugRender();

private:
    void initDebug();

    void InitGBuffer(RenderResource &resource);
};
