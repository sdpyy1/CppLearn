#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Camera.h"

class WindowManager
{
public:
    GLFWwindow* window = nullptr;
    Camera camera;
    int width, height;
    bool mouseHide = true;
#ifdef _WIN32
    int majorVersion = 4;
    int minorVersion = 6;
#endif
#ifdef __APPLE__
    int majorVersion = 3;
    int minorVersion = 3;
#endif
    // 构造函数：初始化宽高和相机
    explicit WindowManager(int w = 800, int h = 600)
            : camera(w, h, glm::vec3(.0f, 5.0f, 10.0f)), width(w), height(h){
        lastX = w/2;
        lastY = h/2;
        firstMouse = true;
        initWindow();
    }
    bool initWindow();

    void processInput();
    static WindowManager* GetInstance(GLFWwindow* window);


private:
    void setDefaultCallbacks() const;
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void APIENTRY myDebugCallback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar* message, const void* userParam)
    {
        // 只打印严重错误
        if (severity == GL_DEBUG_SEVERITY_HIGH)
        {
            std::cerr << "OpenGL Error: " << message << std::endl;
        }
        // 也可以过滤掉 GL_DEBUG_SEVERITY_MEDIUM 等
    }
    // 记录鼠标位置状态等变量可放在这里，或者改成全局静态变量
    bool firstMouse;
    float lastX;
    float lastY;
    // timing
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;
};

#endif
