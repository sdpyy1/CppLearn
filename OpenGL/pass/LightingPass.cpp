#include "LightingPass.h"
#include "../utils/checkGlCommand.h"

LightingPass::LightingPass(Scene &scene)
    : RenderPass("lightingPass"), lightingShader("shader/quad.vert", "shader/lighting.frag"), scene(scene) {
}


void LightingPass::init(RenderResource &resource) {
    // FBO和输出纹理
    glGenFramebuffers(1, &lightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
    glGenTextures(1, &lightTexture);
    glBindTexture(GL_TEXTURE_2D, lightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scene.width, scene.height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[LightingPass] FBO not complete!" << std::endl;
    }
    resource.textures["lightTexture"] = lightTexture;
    resource.framebuffers["lightFBO"] = lightFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderPass::init(resource);
}

void LightingPass::render(RenderResource &resource) {
    if (!isInit) {
        std::cerr << "LightingPass not init!" << std::endl;
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
    glViewport(0, 0, scene.width, scene.height);
    glClear(GL_COLOR_BUFFER_BIT);

    lightingShader.bind();

    // VP
    lightingShader.setMat4("projection", scene.camera->getProjectionMatrix());
    lightingShader.setMat4("view", scene.camera->getViewMatrix());
    // 摄像机
    lightingShader.setVec3("camPos", scene.camera->Position);
    // 灯光
    if (!scene.lights.empty()) {
        lightingShader.setVec3("lightPos", scene.lights[0]->position);
        lightingShader.setVec3("lightColor", scene.lights[0]->color);
    }


    // 转光源视角的矩阵
    lightingShader.setMat4("lightSpaceMatrix", resource.matrices["lightSpaceMatrix"]);

    // 阴影
    lightingShader.setInt("shadowType", scene.shadowType);
    // PCF
    if (scene.shadowType == 2) {
        lightingShader.setInt("pcfScope", scene.pcfScope);
    }
    if (scene.shadowType == 3) {
        lightingShader.setFloat("PCSSBlockerSearchRadius", scene.PCSSBlockerSearchRadius);
        lightingShader.setFloat("PCSSKernelMax", scene.PCSSKernelMax);
        lightingShader.setFloat("PCSSScale", scene.PCSSScale);
    }

    // gBuffer 和 shadowMap
    lightingShader.bindTexture("gPosition",resource.textures["gPosition"],0);
    lightingShader.bindTexture("gNormal",resource.textures["gNormal"],1);
    lightingShader.bindTexture("gAlbedo",resource.textures["gAlbedo"],2);
    lightingShader.bindTexture("gMaterial",resource.textures["gMaterial"],3);
    lightingShader.bindTexture("gEmission",resource.textures["gEmission"],4);
    lightingShader.bindTexture("gDepth",resource.textures["gDepth"],5);
    lightingShader.bindTexture("shadowMap",resource.textures["shadowMap"],6);

    // IBL纹理
    lightingShader.bindCubeMapTexture("irradianceMap",scene.irradianceMap,7);
    lightingShader.bindCubeMapTexture("prefilterMap",scene.prefilterMap,8);
    lightingShader.bindTexture("lutMap",scene.lutMap,9);

    // 渲染
    glBindVertexArray(resource.VAOs["quad"]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    lightingShader.unBind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 用于在后期处理传递
    resource.textures["preTexture"] = lightTexture;
}

