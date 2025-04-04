//
// Created by 刘卓昊 on 2025/4/3.
//
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <valarray>
#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    std::cout << "Failed to create GLFW window" << std::endl;
    glViewport(0, 0, width, height);
}
GLFWwindow * InitWindowAndFunc(){
    glfwInit();
    // 对GLFW的配置 版本号、次版本号、选择核心模式
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }
    // 前两个参数控制窗口左下角的位置。第三个和第四个参数控制渲染窗口的宽度和高度（像素）
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return window;
}

int main()
{
    float vertices[] = {
            0.5f, 0.5f, 0.0f,   // 右上角
            0.5f, -0.5f, 0.0f,  // 右下角
            -0.5f, -0.5f, 0.0f, // 左下角
            -0.5f, 0.5f, 0.0f   // 左上角
    };

    unsigned int indices[] = {
            // 注意索引从0开始!
            // 此例的索引(0,1,2,3)就是顶点数组vertices的下标，
            // 这样可以由下标代表顶点组合成矩形

            0, 1, 3, // 第一个三角形
            1, 2, 3  // 第二个三角形
    };

    GLFWwindow * window = InitWindowAndFunc();

    // 创建一个ID
    GLuint VBO;
    // 此函数会为VBO分配一个未使用的ID（例如 1, 2 等），但此时‌并未实际创建缓冲对象‌，仅预留了标识符
    glGenBuffers(1, &VBO);
    // 绑定到OpenGL上下文中，此时GPU才会真正分配内存
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData是一个专门用来把用户定义的数据复制到当前绑定缓冲的函数。它的第一个参数是目标缓冲的类型：顶点缓冲对象当前绑定到GL_ARRAY_BUFFER目标上。第二个参数指定传输数据的大小(以字节为单位)；用一个简单的sizeof计算出顶点数据大小就行。第三个参数是我们希望发送的实际数据。
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),vertices,GL_STATIC_DRAW);

    // VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // EBO
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    Shader ourShader("./shader/shader.vs", "./shader/shader.fs");


    while(!glfwWindowShouldClose(window))
    {
        // 双缓冲
        glfwSwapBuffers(window);
        // 事件处理
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // 更新uniform颜色
        ourShader.use();
        float timeValue = glfwGetTime();
        float greenValue = sin(timeValue) / 2.0f + 0.5f;
        ourShader.setVec4f("ourColor", 0.0f, greenValue, 0.0f, 1.0f);

        // 通过绑定好的VAO和VBO和EBO画三角形
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glfwTerminate();
    return 0;

}