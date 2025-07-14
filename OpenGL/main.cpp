#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/Shader.h"
#include "core/camera.h"
#include "model/model.h"
#include "core/WindowManager.h"
#include "utils/shaderUtils.h"
#include <iostream>
int main() {
    WindowManager app(800, 600);
    Model model("D:/CppLearn/OpenGL/assets/helmet_pbr/DamagedHelmet.gltf");
    Shader pbrShader("shader/pbr.vert", "shader/pbr.frag");
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(app.window)) {
        app.ProcessInput();

        GL_CALL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        pbrShader.use();
        glm::mat4 modelMat(1.0f);
        handlerShader(pbrShader, app.camera, app.width, app.height, modelMat);
        // Begin Render.
        model.draw(pbrShader);
        // End Render
        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}






