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

#include <iostream>

int main() {
    WindowManager app(800, 600);
    if (!app.Init())
        return -1;

    while (!glfwWindowShouldClose(app.window)) {

        app.ProcessInput();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 这里写渲染逻辑...

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}






