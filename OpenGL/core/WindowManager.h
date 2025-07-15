#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.h"

class WindowManager {
public:
    GLFWwindow* window = nullptr;
    Camera camera;
    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;
    int width, height;

    // 构造函数：初始化宽高和相机
    explicit WindowManager(int w = 800, int h = 600)
            : width(w), height(h), camera(glm::vec3(.0f, .0f, 3.0f)) {
        camera.setWidowSize(width,height);
        Init();
    }

    bool Init();

    void ProcessInput();

    void SetCallbacks();

private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    static WindowManager* GetInstance(GLFWwindow* window);

    // 记录鼠标位置状态等变量可放在这里，或者改成全局静态变量
    static bool firstMouse;
    static float lastX;
    static float lastY;
};

#endif
