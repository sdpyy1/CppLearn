//
// Created by Administrator on 2025/7/25.
//

#include "DebugPass.h"
#include "../core/Shader.h"
#include "../utils/checkGlCommand.h"

DebugPass::DebugPass(Scene &scene) :scene(scene), skyboxShader("shader/skybox.vert", "shader/skybox.frag"),
                                    lightCubeShader("shader/lightCube.vert", "shader/lightCube.frag"),
                                    outlineShader("shader/outline.vert", "shader/outline.frag"){

}

void DebugPass::init(RenderResource& resource){
    passName = "DebugPass";
    RenderPass::init(resource);
}

void DebugPass::render(RenderResource& resource){
    // 天空盒
    skyboxShader.bind();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    skyboxShader.setMat4("projection", scene.camera->getProjectionMatrix());
    skyboxShader.setMat4("view", scene.camera->getViewMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene.envCubemap);
    GL_CALL(scene.renderCube());
    glDepthFunc(GL_LESS);
    skyboxShader.unBind();

    // 描边渲染
    if (scene.selModel && scene.enableOutline) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);  // 关闭深度写入
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);  // 剔除正面，只渲染膨胀模型背面

        outlineShader.bind();
        outlineShader.setMat4("projection", scene.camera->getProjectionMatrix());
        outlineShader.setMat4("view", scene.camera->getViewMatrix());
        outlineShader.setMat4("model", scene.selModel->getModelMatrix());
        outlineShader.setFloat("outlineThickness", 0.01f);
        outlineShader.setVec3("outlineColor", glm::vec3(1.0, 1.0, 0.0));
        scene.selModel->draw(outlineShader);
        outlineShader.unBind();

        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);
    }

    // 光源方块
    if (scene.drawLightCube){
        lightCubeShader.bind();
        lightCubeShader.setMat4("projection", scene.camera->getProjectionMatrix());
        lightCubeShader.setMat4("view", scene.camera->getViewMatrix());
        for (auto& light : scene.lights) {
            auto model = glm::mat4(1.0f);
            if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(scene.selLight)) {
                // 平行光position存储的是方向，而不是位置
                model = glm::translate(model, -light->position);
            }else{
                model = glm::translate(model, light->position);
            }
            model = glm::scale(model, glm::vec3(0.1f)); // 小立方体尺寸
            lightCubeShader.setMat4("model", model);
            lightCubeShader.setVec3("lightColor", light->getFinalColor());
            GL_CALL(scene.renderCube());
        }
        lightCubeShader.unBind();
        glDepthFunc(GL_LESS);
    }
}

