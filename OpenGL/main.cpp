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
#include "pass/RenderPipeline.h"

int main()
{
    // 初始化系统
    WindowManager app(1600, 1200);
    Scene scene(&app.camera);
    std::unique_ptr<RenderPipeline> renderPipeline = RenderPipeline::setupDeferredRenderPipeline(scene);
    ImGUIManger imGUIManger(scene,renderPipeline.get());

    // 搭建场景
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
    DirectionalLight directionalLight(lightDir, glm::vec3(1.0f), 1.0f);
    Model plane = Model::createPlane();
    scene.loadHDRAndIBL("assets/HDR/4.hdr");
    scene.addModel(plane);
    scene.addDefaultModel("helmet");
    Model cube = Model::createCube();
    cube.translation = glm::vec3(2,2,2);
    scene.addModel(cube);
    scene.addLight(std::make_shared<DirectionalLight>(directionalLight));


    while (!glfwWindowShouldClose(app.window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        app.processInput();

        // --- 渲染 Pipeline---
        renderPipeline->render();

        // --- 渲染 ImGui ---
        GL_CALL(imGUIManger.render());

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    // 结束
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
