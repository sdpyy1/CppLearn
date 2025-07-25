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

void printMatrix(const glm::mat4& mat, const std::string& name) {
    std::cout << name << " = \n" << glm::to_string(mat) << std::endl;
}
ShadowPass::ShadowPass(Scene &scene): scene(scene), shadowShader("shader/shadow.vert", "shader/shadow.frag"), debugShader("shader/debugDepthMap.vert", "shader/debugDepthMap.frag")
{
}

void ShadowPass::init(RenderResource& resource) {
    passName = "ShadowPass";
    glGenFramebuffers(1, &shadowFBO);
    // 创建深度纹理
    glGenTextures(1, &shadowMap);
    resource.framebuffers["shadowFBO"] = shadowFBO;
    resource.textures["shadowMap"] = shadowMap;
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 scene.width, scene.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
    initDebug();
    RenderPass::init(resource);
}

void ShadowPass::render(RenderResource& resource) {
    if (!isInit) {
        std::cout << "shadowPass init" << std::endl;
        return;
    }

    glViewport(0, 0, scene.width, scene.height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // 可选：开启面剔除，防止 Peter-panning
    // glCullFace(GL_FRONT);

    shadowShader.bind();

    // 1. 准备光源的正交投影和视图矩阵
    float orthoSize = 100.0f;  // 根据场景大小调整
    float near_plane = 1.f;
    float far_plane = 30.0f;

    // 正交投影，范围是正交体体积，覆盖阴影区域
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);

    // 计算光的视图矩阵
    // 这里光源方向是scene.light.position（其实是方向），假设是单位向量
    glm::vec3 lightDir = glm::normalize(scene.lights[0]->position);

    // 为了构造视图矩阵，需要光源的位置和目标点
    // 假设光源“位置”在lightDir反方向的某个远点
    glm::vec3 lightPos = -lightDir * 20.0f;  // 20可调整，确保覆盖场景
    glm::vec3 target = glm::vec3(0.0f);      // 观察点：通常场景中心
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 lightView = glm::lookAt(lightPos, target, up);

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


void ShadowPass::initDebug(){
    float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &debugQuadVAO);
    glGenBuffers(1, &debugQuadVBO);
    glBindVertexArray(debugQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
};

void ShadowPass::renderDebug() {
    debugShader.bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    debugShader.setInt("depthMap", 0);
    glBindVertexArray(debugQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    debugShader.unBind();
    glBindTexture(GL_TEXTURE_2D, 0);
}