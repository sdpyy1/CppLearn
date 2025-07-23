#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "core/Shader.h"
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "pass/GeometryPass.h"
#include "pass/LightingPass.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "helper/ImGUIManger.h"

int main()
{
    WindowManager app(2560, 1920);
    Scene scene(&app.camera);
    ImGUIManger imGUIManger(scene);
    // 加载Shader
    Shader skyboxShader("shader/skybox.vert", "shader/skybox.frag");
    Shader lightCubeShader("shader/lightCube.vert", "shader/lightCube.frag");
    Shader outlineShader("shader/outline.vert", "shader/outline.frag");
    // 搭建场景
    PointLight pointLight(glm::vec3(.0f, .0f, -5.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
    Model plane = Model::createPlane();
    scene.addModel(plane);
    scene.addLight(std::make_shared<PointLight>(pointLight));


    // 初始化Pass
    GeometryPass geometryPass(scene);
    geometryPass.init();
    LightingPass lighting(scene, geometryPass.gPosition, geometryPass.gNormal, geometryPass.gAlbedo,geometryPass.gMaterial, geometryPass.gEmission,geometryPass.gDepth);
    lighting.init();

    while (!glfwWindowShouldClose(app.window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        app.processInput();

        // G-Buffer Pass
        geometryPass.render();
//        geometryPass.debugRender();

        // light Pass
        lighting.lightingShader.bind();
        glEnable(GL_DEPTH_TEST);
        // IBL开启
        glActiveTexture(GL_TEXTURE15);glBindTexture(GL_TEXTURE_CUBE_MAP, scene.irradianceMap);lighting.lightingShader.setInt("irradianceMap", 15);
        glActiveTexture(GL_TEXTURE14);glBindTexture(GL_TEXTURE_CUBE_MAP, scene.prefilterMap);lighting.lightingShader.setInt("prefilterMap", 14);
        glActiveTexture(GL_TEXTURE13);glBindTexture(GL_TEXTURE_2D, scene.lutMap);lighting.lightingShader.setInt("lutMap", 13);
        GL_CALL(lighting.render());

        lighting.lightingShader.unBind();


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
                model = glm::translate(model, light->position);
                model = glm::scale(model, glm::vec3(0.1f)); // 小立方体尺寸
                lightCubeShader.setMat4("model", model);
                lightCubeShader.setVec3("lightColor", light->getFinalColor());
                GL_CALL(scene.renderCube());
            }
            lightCubeShader.unBind();
            glDepthFunc(GL_LESS);
        }

        // --- 渲染 ImGui ---
        imGUIManger.render();


        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
