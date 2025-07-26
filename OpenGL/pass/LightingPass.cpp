#include "LightingPass.h"
#include "../utils/checkGlCommand.h"

LightingPass::LightingPass(Scene &scene)
        : RenderPass("lightingPass"),lightingShader("shader/pbr_gBuffer.vert", "shader/pbr_gBuffer.frag"), scene(scene)
{}



void LightingPass::init(RenderResource& resource) {
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

void LightingPass::render(RenderResource& resource) {
    if (!isInit){
        std::cerr << "LightingPass not init!" << std::endl;
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
    glViewport(0, 0, scene.width, scene.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GL_CALL(lightingShader.bind());

    // VP
    lightingShader.setMat4("projection", scene.camera->getProjectionMatrix());
    lightingShader.setMat4("view", scene.camera->getViewMatrix());

    //SSR
    lightingShader.setInt("EnableSSR", scene.EnableSSR?1:0);
    lightingShader.setInt("totalStepTimes",scene.totalStepTimes);
    lightingShader.setFloat("stepSize",scene.stepSize);
    lightingShader.setFloat("EPS",scene.EPS);
    lightingShader.setFloat("threshold",scene.threshold);
    lightingShader.setFloat("SSRStrength",scene.SSRStrength);

    // gBuffer 和 shadowMap
    GL_CALL(lightingShader.setInt("gPosition", 0));
    GL_CALL(lightingShader.setInt("gNormal", 1));
    GL_CALL(lightingShader.setInt("gAlbedo", 2));
    GL_CALL(lightingShader.setInt("gMaterial", 3));
    GL_CALL(lightingShader.setInt("gEmission", 4));
    GL_CALL(lightingShader.setInt("gDepth", 5));
    GL_CALL(lightingShader.setInt("shadowMap", 6));
    lightingShader.setMat4("lightSpaceMatrix", resource.matrices["lightSpaceMatrix"]);

    // 摄像机
    GL_CALL(lightingShader.setVec3("camPos", scene.camera->Position));

    // 灯光
    if (!scene.lights.empty()) {
        GL_CALL(lightingShader.setVec3("lightPos", scene.lights[0]->position));
        GL_CALL(lightingShader.setVec3("lightColor", scene.lights[0]->color));
    }

    // 阴影
    lightingShader.setInt("shadowType", scene.shadowType);
    // PCF
    if (scene.shadowType == 2){
        lightingShader.setInt("pcfScope", scene.pcfScope);
    }
    if (scene.shadowType == 3){
        lightingShader.setFloat("PCSSBlockerSearchRadius", scene.PCSSBlockerSearchRadius);
        lightingShader.setFloat("PCSSKernelMax", scene.PCSSKernelMax);
        lightingShader.setFloat("PCSSScale", scene.PCSSScale);
    }

    // 绑定 G-Buffer
    GL_CALL(glActiveTexture(GL_TEXTURE0));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gPosition"]));
    GL_CALL(glActiveTexture(GL_TEXTURE1));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gNormal"]));
    GL_CALL(glActiveTexture(GL_TEXTURE2));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gAlbedo"]));
    GL_CALL(glActiveTexture(GL_TEXTURE3));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gMaterial"]));
    GL_CALL(glActiveTexture(GL_TEXTURE4));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gEmission"]));
    GL_CALL(glActiveTexture(GL_TEXTURE5));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gDepth"]));
    GL_CALL(glActiveTexture(GL_TEXTURE6));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["shadowMap"]));

    // IBL纹理
    glActiveTexture(GL_TEXTURE15);glBindTexture(GL_TEXTURE_CUBE_MAP, scene.irradianceMap);lightingShader.setInt("irradianceMap", 15);
    glActiveTexture(GL_TEXTURE14);glBindTexture(GL_TEXTURE_CUBE_MAP, scene.prefilterMap);lightingShader.setInt("prefilterMap", 14);
    glActiveTexture(GL_TEXTURE13);glBindTexture(GL_TEXTURE_2D, scene.lutMap);lightingShader.setInt("lutMap", 13);
    GL_CALL(glBindVertexArray(resource.VAOs["quad"]));
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_CALL(glBindVertexArray(0));
    GL_CALL(lightingShader.unBind());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

