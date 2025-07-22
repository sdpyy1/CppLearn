#include "WindowManager.h"
#include <iostream>


bool WindowManager::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, "OpenGL Window", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }
    glfwSetWindowUserPointer(window, this);

    setDefaultCallbacks();

    glViewport(0, 0, width, height);

    // 不显示鼠标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(myDebugCallback, nullptr);
    return true;
}

void WindowManager::processInput()
{
    auto currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}

void WindowManager::setDefaultCallbacks() const {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
}

WindowManager* WindowManager::GetInstance(GLFWwindow* window) {
    return reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
}

void WindowManager::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    WindowManager* app = GetInstance(window);
    app->camera.height = width;
    app->camera.width = height;
    app->width = width;
    app->height = height;
    glViewport(0, 0, width, height);

}

void WindowManager::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    WindowManager* app = GetInstance(window);
    if (!app) {
        std::cout << "WindowManager::mouse_callback: Invalid window" << std::endl;
        return;
    }

    if (app->firstMouse) {
        app->lastX = static_cast<float>(xpos);
        app->lastY = static_cast<float>(ypos);
        app->firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - app->lastX;
    float yoffset = app->lastY - static_cast<float>(ypos); // reversed

    app->lastX = static_cast<float>(xpos);
    app->lastY = static_cast<float>(ypos);

    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

void WindowManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    WindowManager* app = GetInstance(window);
    if (!app) return;
    app->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
void WindowManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    WindowManager* app = GetInstance(window);
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        if (app->mouseHide) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetScrollCallback(window, nullptr);
            glfwSetCursorPosCallback(window, nullptr);
            app->mouseHide = false;

            // 重置鼠标位置，避免大偏移
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->lastX = static_cast<float>(xpos);
            app->lastY = static_cast<float>(ypos);
            app->firstMouse = true;  // 重新开始鼠标偏移计算
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetScrollCallback(window, scroll_callback);
            glfwSetCursorPosCallback(window, mouse_callback);
            app->mouseHide = true;

            // 隐藏鼠标时也重置鼠标位置缓存
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->lastX = static_cast<float>(xpos);
            app->lastY = static_cast<float>(ypos);
            app->firstMouse = true;
        }
    }
}