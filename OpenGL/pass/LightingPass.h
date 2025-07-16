#pragma once
#include <glad/glad.h>
#include <vector>
#include "../core/Shader.h"
#include "RenderPass.h"
#include "../helper/Scene.h"

class LightingPass : public RenderPass {
public:
    LightingPass(Scene &scene, GLuint gPosition, GLuint gNormal, GLuint gAlbedo, GLuint gMaterial);

    void init() override;
    void render() override;

private:
    void initQuad();

    Shader lightingShader;
    GLuint quadVAO;
    GLuint gPosition, gNormal, gAlbedo, gMaterial;
    Scene &scene;
};
