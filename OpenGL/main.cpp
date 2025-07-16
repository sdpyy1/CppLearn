#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "core/Shader.h"
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "pass/RenderPass.h"
#include "pass/GeometryPass.h"
#include "pass/LightingPass.h"
#include <iostream>

int main()
{
    WindowManager app(1280, 1280);
    Scene scene(&app.camera);
    // Model model("assets/desert-eagle/scene.gltf");
    Model model("assets/helmet_pbr/DamagedHelmet.gltf");
    //    Model model("C:/Users/Administrator/Desktop/未命名.obj");
    //    Model model("D:/CppLearn/OpenGL/assets/backpack/backpack.obj");
    PointLight pointLight(glm::vec3(.0f, .0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
    PointLight pointLight1(glm::vec3(.0f, .0f, -4.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
    PointLight pointLight2(glm::vec3(.0f, 4.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
    scene.addModel(model);
    scene.addLight(std::make_shared<PointLight>(pointLight));
    scene.addLight(std::make_shared<PointLight>(pointLight1));
    scene.addLight(std::make_shared<PointLight>(pointLight2));
    Shader pbrShader("shader/pbr.vert", "shader/pbr.frag");
    GeometryPass geometryPass(scene);
    geometryPass.init();
    LightingPass lighting(scene,geometryPass.gPosition, geometryPass.gNormal, geometryPass.gAlbedo, geometryPass.gMaterial);
    lighting.init();

    while (!glfwWindowShouldClose(app.window))
    {
        app.processInput();
        GL_CALL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        // G-Buffer Pass
        geometryPass.render();

        geometryPass.debug();
        // PBR材质渲染
        pbrShader.use();
        scene.drawAll(pbrShader);
//        lighting.render();

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}
