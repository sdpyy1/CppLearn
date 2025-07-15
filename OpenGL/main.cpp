#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "core/Shader.h"
#include "model/Model.h"
#include "core/WindowManager.h"
#include "helper/Scene.h"
#include "pass/RenderPass.h"
#include "pass/GeometryPass.h"
#include <iostream>

int main()
{
    WindowManager app(1280, 1280);
    Scene scene(&app.camera);
    // Model model("assets/desert-eagle/scene.gltf");
    Model model("assets/helmet_pbr/DamagedHelmet.gltf");
    //    Model model("C:/Users/Administrator/Desktop/未命名.obj");
    //    Model model("D:/CppLearn/OpenGL/assets/backpack/backpack.obj");

    scene.addModel(model);
    Shader pbrShader("shader/pbr.vert", "shader/pbr.frag");
    GeometryPass geometryPass(scene);
    geometryPass.init();

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
        glEnable(GL_DEPTH_TEST);
        scene.drawAll(pbrShader);
        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}
