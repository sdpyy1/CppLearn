#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "helper/ImGUIManger.h"
#include "RenderPipeline.h"
#include "glm/gtx/string_cast.hpp"

int main() {
    // 初始化系统
    WindowManager app(1920, 1680);

    // 搭建场景
    Scene scene(&app.camera);
    // 太阳光
    glm::vec3 Sun = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
    DirectionalLight directionalLight(Sun, glm::vec3(1.0f), 1.0f);
    scene.addLight(std::make_shared<DirectionalLight>(directionalLight));
    // 模型
    Model plane = Model::createPlane(100, 1);
    scene.loadHDRAndIBL("assets/HDR/2.hdr");
    scene.addModel(plane);
    scene.addDefaultModel("helmet");

    // 管线
    RenderPipeline renderPipeline;
    renderPipeline.setupDeferredRenderPipeline(scene);

    // GUI
    ImGUIManger imGUIManger(scene, &renderPipeline);

    // 实验
    // glm::vec4 pos = {1,3,4,0};
    // auto transtion = glm::mat4(1.0);
    // std::cout << glm::to_string(app.camera.getProjectionMatrix()*app.camera.getViewMatrix()*transtion*pos) << std::endl;

    // 渲染
    while (!glfwWindowShouldClose(app.window)) {
        app.processInput();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // --- 渲染 Pipeline---
        renderPipeline.render();

        // --- 渲染 ImGui ---
        imGUIManger.render();

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
