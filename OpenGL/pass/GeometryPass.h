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
    GLuint debugVAO;
    Scene& scene;
    int width, height;
    GLuint gBuffer;

public:
    GLuint gPosition, gNormal, gAlbedo, gMaterial, gDepth;

    explicit GeometryPass(Scene& scene);


    void init() override;
    void render() override;

    void debug();

private:
    void InitGBuffer();
    void initDebug();
};
