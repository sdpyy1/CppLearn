#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "core/Shader.h"
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "pass/GeometryPass.h"
#include "pass/LightingPass.h"
#include "precompute/preComputer.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

int main()
{
    WindowManager app(2560, 1920);

    Scene scene(&app.camera);
    // 加载Shader
    Shader skyboxShader("shader/skybox.vert", "shader/skybox.frag");

    // 搭建场景
    PointLight pointLight(glm::vec3(.0f, .0f, -5.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
//    scene.addDefaultModel("helmet");
    scene.addDefaultModel("gun");
//    scene.addModel( "assets/asw/scene.gltf");
//    scene.addLight(std::make_shared<PointLight>(pointLight));

    // IBL
    // TODO:cubemap加载目前必须放在模型加载之后
    scene.loadCubemapFromHDR("assets/HDR/4.hdr");
//    scene.loadCubemapFromSkybox("assets/cubemap/Skybox");
    preComputer preComputer(scene);
    GLuint irradianceMap = preComputer.computeIrradianceMap();
    GLuint prefilterMap = preComputer.computePrefilterMap();
    GLuint lutMap = preComputer.computeLutMap();

    // 初始化Pass
    GeometryPass geometryPass(scene);
    geometryPass.init();
    LightingPass lighting(scene, geometryPass.gPosition, geometryPass.gNormal, geometryPass.gAlbedo,geometryPass.gMaterial, geometryPass.gEmission,geometryPass.gDepth);
    lighting.init();

    while (!glfwWindowShouldClose(app.window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("My Debug Window");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Hello, world!");
        if (ImGui::Button("left Click btn"))
        {
            // 按钮被点击时执行的代码
            std::cout << "Button clicked!" << std::endl;
        }
//        ImGui::SetWindowPos({100,100});
        ImVec2 nSize = { 300, 200 };
        ImGui::SetWindowSize(nSize);
        static float lightIntensity = 1.0f;
        ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 10.0f);
        ImGui::End();
        app.processInput();

        // G-Buffer Pass
        geometryPass.render();

        // light Pass
        lighting.lightingShader.bind();
        glEnable(GL_DEPTH_TEST);
        // IBL开启
        glActiveTexture(GL_TEXTURE15);glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);lighting.lightingShader.setInt("irradianceMap", 15);
        glActiveTexture(GL_TEXTURE14);glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);lighting.lightingShader.setInt("prefilterMap", 14);
        glActiveTexture(GL_TEXTURE13);glBindTexture(GL_TEXTURE_2D, lutMap);lighting.lightingShader.setInt("lutMap", 13);
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

        // --- 渲染 ImGui ---
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
