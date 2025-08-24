//
// Created by Administrator on 2025/7/25.
//

#include "DebugPass.h"
#include "../../core/Shader.h"
#include "../../utils/checkGlCommand.h"

DebugPass::DebugPass(Scene &scene) : RenderPass("DebugPass"), scene(scene),
                                     skyboxShader("shader/debug/skybox.vert", "shader/debug/skybox.frag"),
                                     lightCubeShader("shader/debug/lightCube.vert", "shader/debug/lightCube.frag"),
                                     outlineShader("shader/debug/outline.vert", "shader/debug/outline.frag") {
}

void DebugPass::init(RenderResource &resource) {
    RenderPass::init(resource);
}

void DebugPass::render(RenderResource &resource) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 天空盒  整合到体积云中
    // if (scene.showSkybox) {
    //     skyboxShader.bind();
    //     glEnable(GL_DEPTH_TEST);
    //     glDepthMask(GL_FALSE);
    //     // 小于等于都通过测试，因为天空盒z永远=1，所以没有模型的地方肯定会通过测试
    //     glDepthFunc(GL_LEQUAL);
    //     skyboxShader.setMat4("projection", scene.camera->getProjectionMatrix());
    //     skyboxShader.setMat4("view", scene.camera->getViewMatrix());
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_CUBE_MAP, scene.envCubemap);
    //     scene.renderCube();
    //     glDepthFunc(GL_LESS);
    //     glDisable(GL_DEPTH_TEST);
    //     glDepthMask(GL_TRUE);
    //     skyboxShader.unBind();
    // }


    // 描边渲染
    if (scene.selModel && scene.enableOutline) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE); // 关闭深度写入
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT); // 剔除正面，只渲染膨胀模型背面
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
        glDisable(GL_DEPTH_TEST);
    }
    // 光源方块
    if (scene.drawLightCube) {
        lightCubeShader.bind();
        lightCubeShader.setMat4("projection", scene.camera->getProjectionMatrix());
        lightCubeShader.setMat4("view", scene.camera->getViewMatrix());
        for (auto &light: scene.lights) {
            auto model = glm::mat4(1.0f);
            if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(scene.selLight)) {
                // 平行光position存储的是方向，而不是位置
                model = glm::translate(model, -light->position * 10.0f);
            } else {
                model = glm::translate(model, light->position);
            }
            model = glm::scale(model, glm::vec3(0.1f)); // 小立方体尺寸
            lightCubeShader.setMat4("model", model);
            lightCubeShader.setVec3("lightColor", light->getFinalColor());
            GL_CALL(scene.renderCube());
        }
        lightCubeShader.unBind();
    }
    if (scene.showDebugTexture) {
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(resource.VAOs["quad"]);
        resource.shaders["quadShader"].get()->bind();
        glDepthMask(GL_FALSE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const int maxPerRow = 2;
        int totalSelected = 0;
        std::vector<std::tuple<GLuint, int, std::string> > selectedTextures;

        // 收集选中的纹理（逻辑不变）
        auto addIfSelected = [&](bool isSelected, GLuint texId, int channel, const std::string &name) {
            if (isSelected && totalSelected < 4) {
                selectedTextures.emplace_back(texId, channel, name);
                totalSelected++;
            }
        };

        // 添加选中的纹理（按顺序）
        addIfSelected(scene.showAlbedo, resource.textures["gAlbedo"], 0, "Albedo");
        addIfSelected(scene.showNormal, resource.textures["gNormal"], 5, "Normal");
        addIfSelected(scene.showPosition, resource.textures["gPosition"], 0, "Position");
        addIfSelected(scene.showDepth, resource.textures["gDepth"], 6, "Depth");
        addIfSelected(scene.showShadowMap, resource.textures["shadowMap"], 1, "ShadowMap");
        addIfSelected(scene.showMetallic, resource.textures["gMaterial"], 1, "Metallic");
        addIfSelected(scene.showRoughness, resource.textures["gMaterial"], 2, "Roughness");
        addIfSelected(scene.showAO, resource.textures["gMaterial"], 3, "AO");
        addIfSelected(scene.showEmission, resource.textures["gEmission"], 0, "Emission");
        addIfSelected(scene.showLightTexture, resource.textures["lightTexture"], 0, "lightTexture");

        // 关键修复：处理 totalSelected = 0 的情况
        if (totalSelected > 0) {
            // 只有选中纹理时才计算网格并绘制
            // 计算行数（向上取整，避免除以零）
            int rows = (totalSelected + maxPerRow - 1) / maxPerRow;
            // 计算网格尺寸（此时 rows 至少为1，不会除以零）
            int gridW = scene.width / maxPerRow;
            int gridH = scene.height / rows;

            // 绘制选中的纹理
            for (int i = 0; i < totalSelected; i++) {
                auto [texId, channel, name] = selectedTextures[i];
                int col = i % maxPerRow; // 列索引（0或1）
                int row = i / maxPerRow; // 行索引（0或1，因为最多4个）

                // 计算视口位置
                int x = col * gridW;
                int y = row * gridH; // 注意：OpenGL视口Y轴向下，如需翻转可调整为 (rows - 1 - row) * gridH

                glViewport(x, y, gridW, gridH);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texId);

                // 设置shader参数
                resource.shaders["quadShader"].get()->setInt("quadShader", 0);
                resource.shaders["quadShader"].get()->setInt("channel", channel);
                if (name == "Depth") {
                    resource.shaders["quadShader"].get()->setFloat("near", scene.camera->Near);
                    resource.shaders["quadShader"].get()->setFloat("far", scene.camera->Far);
                }

                GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
            }
        }

        // 恢复状态
        GL_CALL(glBindVertexArray(0));
        GL_CALL(glViewport(0, 0, scene.width, scene.height));
        GL_CALL(glEnable(GL_DEPTH_TEST));
        glDepthMask(GL_TRUE);
        resource.shaders["quadShader"].get()->unBind();
    }
}

