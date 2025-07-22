#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"

class WindowManager
{
public:
    GLFWwindow* window = nullptr;
    Camera camera;

    int width, height;
    bool mouseHide = true;


    // 构造函数：初始化宽高和相机
    explicit WindowManager(int w = 800, int h = 600)
            : camera(w, h, glm::vec3(.0f, 3.0f, 15.0f)), width(w), height(h){
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

    // 记录鼠标位置状态等变量可放在这里，或者改成全局静态变量
    bool firstMouse;
    float lastX;
    float lastY;
    // timing
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;
};

#endif
