//
// Created by Administrator on 2025/7/24.
//
#include <iostream>
#include "ShadowPass.h"
#include "../helper/Scene.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <iostream>
ShadowPass::ShadowPass(Scene &scene): RenderPass("ShadowPass"),scene(scene), shadowShader("shader/shadow.vert", "shader/shadow.frag")
{
}

void ShadowPass::init(RenderResource& resource) {
    glGenFramebuffers(1, &shadowFBO);
    // 创建深度纹理
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach 深度纹理到FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    // 不需要渲染颜色，只写深度
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    resource.framebuffers["shadowFBO"] = shadowFBO;
    resource.textures["shadowMap"] = shadowMap;
    RenderPass::init(resource);
}

void ShadowPass::render(RenderResource& resource) {
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // TODO:这里得查一下为什么可以防止
    // 可选：开启面剔除，防止 Peter-panning
    // glCullFace(GL_FRONT);
    shadowShader.bind();

    // 正交投影，范围是正交体体积，覆盖阴影区域
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);

    glm::mat4 lightView = glm::lookAt(-glm::normalize(scene.lights[0]->position)*10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 计算光空间矩阵
    lightSpaceMatrix = lightProjection * lightView;

    // 传给shader
    shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    resource.matrices["lightSpaceMatrix"] = lightSpaceMatrix;
    // 2. 渲染场景中所有模型的深度
    for (auto& model : scene.models) {
        glm::mat4 modelMatrix = model.getModelMatrix();
        shadowShader.setMat4("model", modelMatrix);
        model.draw(shadowShader);
    }

    shadowShader.unBind();

    // 恢复默认面剔除（如果之前修改过）
    // glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 恢复视口为窗口大小
    glViewport(0, 0, scene.width, scene.height);
}