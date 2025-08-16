//
// Created by Administrator on 2025/7/26.
//

#include "PostprocessPass.h"
#include "../../utils/checkGlCommand.h"

PostprocessPass::PostprocessPass(Scene &scene, const Shader &postShader): RenderPass("Postprocess_NUKOWN"),
                                                                          scene(scene), postShader(postShader) {
}

void PostprocessPass::toScreen() {
    if (postFBO != 0) {
        glDeleteFramebuffers(1, &postFBO);
        postFBO = 0;
    }
}

void PostprocessPass::init(RenderResource &resource) {
    isRender = true;
    // 初始化FBO
    glGenFramebuffers(1, &postFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
    // 绑定输出纹理
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, scene.width, scene.height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
    resource.textures[passName] = renderedTexture;
    RenderPass::init(resource);
}

void PostprocessPass::render(RenderResource &resource) {
    if (isRender) {
        glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(resource.VAOs["quad"]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        postShader.unBind();
        // 给下一个pass传递上一个pass的结果
        resource.textures["preTexture"] = renderedTexture;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
}

int PostprocessPass::bindParams(RenderResource &resource) {
    postShader.bind();
    // VP
    postShader.setMat4("projection", scene.camera->getProjectionMatrix());
    postShader.setMat4("view", scene.camera->getViewMatrix());
    // 摄像机
    postShader.setVec3("camPos", scene.camera->Position);
    // 灯光
    if (!scene.lights.empty()) {
        postShader.setVec3("lightPos", scene.lights[0]->position);
        postShader.setVec3("lightColor", scene.lights[0]->color);
    }
    postShader.bindTexture("gPosition",resource.textures["gPosition"],0);
    postShader.bindTexture("gNormal",resource.textures["gNormal"],1);
    postShader.bindTexture("gAlbedo",resource.textures["gAlbedo"],2);
    postShader.bindTexture("gMaterial",resource.textures["gMaterial"],3);
    postShader.bindTexture("gEmission",resource.textures["gEmission"],4);
    postShader.bindTexture("gDepth",resource.textures["gDepth"],5);
    postShader.bindTexture("shadowMap",resource.textures["shadowMap"],6);

    postShader.bindTexture("lightTexture",resource.textures["lightTexture"],7);
    postShader.bindTexture("preTexture",resource.textures["preTexture"],8);
    // 返回下一个可用的纹理通道
    return 9;
}
