#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "core/Shader.h"
#include "model/model.h"
#include "core/WindowManager.h"
#include "utils/shaderUtils.h"
#include "utils/DefaultSceneSetup.h"
#include <iostream>
int main() {
    WindowManager app(1920, 1400);
    Model model("D:/CppLearn/OpenGL/assets/desert-eagle/scene.gltf");
    Shader pbrShader("shader/pbr.vert", "shader/pbr.frag");
    Shader basicShader("shader/basic.vert", "shader/basic.frag");
    glEnable(GL_DEPTH_TEST);
    // 搭建一个基础场景
    DefaultSceneSetup scene(app.camera, app.width, app.height);
    while (!glfwWindowShouldClose(app.window)) {
        app.ProcessInput();
        GL_CALL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        // 基础渲染
        basicShader.use();
        scene.draw(basicShader);
        // PBR材质渲染
        pbrShader.use();
        handlerShader(pbrShader, app.camera, app.width, app.height, glm::mat4(0.2f));
        model.draw(pbrShader);
        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}






