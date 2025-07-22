#pragma once
#include <glad/glad.h>
#include <vector>
#include "../core/Shader.h"
#include "RenderPass.h"
#include "../helper/Scene.h"

class LightingPass : public RenderPass {
public:
    LightingPass(Scene &scene, GLuint gPosition, GLuint gNormal, GLuint gAlbedo, GLuint gMaterial, GLuint gEmission,GLuint gDepth);

    void init() override;
    void render() override;
    Shader lightingShader;
private:
    void initQuad();


    GLuint quadVAO;
    GLuint gPosition, gNormal, gAlbedo, gMaterial, gEmission,gDepth;
    Scene &scene;
};
