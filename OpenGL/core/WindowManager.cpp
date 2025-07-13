#include "WindowManager.h"
#include <iostream>

bool WindowManager::firstMouse = true;
float WindowManager::lastX = 400.0f;
float WindowManager::lastY = 300.0f;

bool WindowManager::Init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
    glfwSetWindowUserPointer(window, this);  // ✅ 这一句非常重要！

    SetCallbacks();

    glViewport(0, 0, width, height);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return true;
}

void WindowManager::ProcessInput() {
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

void WindowManager::SetCallbacks() {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

WindowManager* WindowManager::  GetInstance(GLFWwindow* window) {
    return reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
}

void WindowManager::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void WindowManager::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    WindowManager* app = GetInstance(window);

    if (!app) {
        std::cout << "WindowManager::mouse_callback: Invalid window" << std::endl;
        return;
    }

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

void WindowManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    WindowManager* app = GetInstance(window);
    if (!app) return;

    app->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
