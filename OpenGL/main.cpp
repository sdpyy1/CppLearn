//
// Created by 刘卓昊 on 2025/4/3.
//
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include "macros.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
GLFWwindow * InitWindowAndFunc();
void prepareData(GLuint &cubeVAO, GLuint &lightVAO);
void setMVP(const Shader &aShader, glm::mat4 &modelTrans);
GLuint loadTexture(char const * path);

void drawCube(const Shader &cubeShader, GLuint cubeVAO, GLuint diffuseTexture, GLuint specularTexture);

void drawLight(const Shader &lightingShader, GLuint lightVAO);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;




int main(){
    // 初始化窗口
    GLFWwindow * window = InitWindowAndFunc();
    // 着色器设置
    Shader cubeShader("./shader/cube_shader.vert", "./shader/cube_shader.frag");
    Shader lightingShader("./shader/light_shader.vert", "./shader/light_shader.frag");
    // 获取到各种的VAO
    GLuint cubeVAO, lightVAO;
    prepareData(cubeVAO, lightVAO);

    // 设置纹理
    GLuint diffuseTexture = loadTexture("./assets/diffuse.png");
    GLuint specularTexture = loadTexture("./assets/specular.png");

    // 初始化时启用深度测试
    GL_CALL(glEnable(GL_DEPTH_TEST));


    while (!glfwWindowShouldClose(window))
    {
        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        // 清理窗口
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        // 画箱子
        drawCube(cubeShader, cubeVAO, diffuseTexture, specularTexture);
        // 画光源
        drawLight(lightingShader, lightVAO);

        // 事件处理
        glfwPollEvents();
        // 双缓冲
        glfwSwapBuffers(window);

    }
    glfwTerminate();
    return 0;
}

void drawLight(const Shader &lightingShader, GLuint lightVAO) {// 画光源
    // 点光源位置
    glm::vec3 pointLightPositions[] = {
            glm::vec3( 0.7f,  0.2f,  2.0f),
            glm::vec3( 2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3( 0.0f,  0.0f, -3.0f)
    };
    GL_CALL(glBindVertexArray(lightVAO));
    lightingShader.use();
    for (unsigned int i = 0; i < 4; i++)
    {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[i]);
        model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        setMVP(lightingShader, model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void drawCube(const Shader &cubeShader, GLuint cubeVAO, GLuint diffuseTexture, GLuint specularTexture) {// 画正方形
    glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    // 点光源位置
    glm::vec3 pointLightPositions[] = {
            glm::vec3( 0.7f,  0.2f,  2.0f),
            glm::vec3( 2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3( 0.0f,  0.0f, -3.0f)
    };
    GL_CALL(glBindVertexArray(cubeVAO));
    cubeShader.use();
    cubeShader.setVec3("viewPos", camera.Position);
//    cubeShader.setVec3("light.position", lightPos);
    // Material properties
    GL_CALL(glActiveTexture(GL_TEXTURE0));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, diffuseTexture));
    GL_CALL(glActiveTexture(GL_TEXTURE1));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, specularTexture));
    cubeShader.setInt("material.diffuse", 0);
    cubeShader.setInt("material.specular", 1);
    cubeShader.setFloat("material.shininess", 32.0f);
    // light properties
    // directional light
    cubeShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    cubeShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    cubeShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    cubeShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    cubeShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    cubeShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    cubeShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    cubeShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    cubeShader.setFloat("pointLights[0].constant", 1.0f);
    cubeShader.setFloat("pointLights[0].linear", 0.09f);
    cubeShader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    cubeShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    cubeShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    cubeShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    cubeShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    cubeShader.setFloat("pointLights[1].constant", 1.0f);
    cubeShader.setFloat("pointLights[1].linear", 0.09f);
    cubeShader.setFloat("pointLights[1].quadratic", 0.032f);
    // point light 3
    cubeShader.setVec3("pointLights[2].position", pointLightPositions[2]);
    cubeShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    cubeShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    cubeShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    cubeShader.setFloat("pointLights[2].constant", 1.0f);
    cubeShader.setFloat("pointLights[2].linear", 0.09f);
    cubeShader.setFloat("pointLights[2].quadratic", 0.032f);
    // point light 4
    cubeShader.setVec3("pointLights[3].position", pointLightPositions[3]);
    cubeShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    cubeShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    cubeShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    cubeShader.setFloat("pointLights[3].constant", 1.0f);
    cubeShader.setFloat("pointLights[3].linear", 0.09f);
    cubeShader.setFloat("pointLights[3].quadratic", 0.032f);
    // spotLight
    cubeShader.setVec3("spotLight.position", camera.Position);
    cubeShader.setVec3("spotLight.direction", camera.Front);
    cubeShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    cubeShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    cubeShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    cubeShader.setFloat("spotLight.constant", 1.0f);
    cubeShader.setFloat("spotLight.linear", 0.09f);
    cubeShader.setFloat("spotLight.quadratic", 0.032f);
    cubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    cubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
    for(unsigned int i = 0; i < 10; i++)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        setMVP(cubeShader,model);
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 36));
    }
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 36));
}

void setMVP(const Shader &aShader, glm::mat4 & modelTrans) {
    // 构建MVP矩阵
    auto model = modelTrans;
    auto view = camera.GetViewMatrix();
    auto projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    aShader.setMat4("model", model);
    aShader.setMat4("view", view);
    aShader.setMat4("projection", projection);
}


void prepareData(GLuint &cubeVAO, GLuint &lightVAO) {
    float vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // 创建VBO的ID
    GLuint VBO;

    GL_CALL(glGenBuffers(1, &VBO));

    // VBO写入显存
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // 创建VAO的ID
    GL_CALL(glGenVertexArrays(1, &cubeVAO));
    GL_CALL(glGenVertexArrays(1,&lightVAO));


    // 绑定VAO到正方形
    GL_CALL(glBindVertexArray(cubeVAO));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0));
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CALL(glEnableVertexAttribArray(1));
    GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))));
    GL_CALL(glEnableVertexAttribArray(2));

    // 绑定VAO到灯光
    GL_CALL(glBindVertexArray(lightVAO));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
    GL_CALL(glEnableVertexAttribArray(0));

    // 解绑VAO，在渲染循环中，需要哪一个再绑定哪一个
    GL_CALL(glBindVertexArray(0));
    // VBO已经被设置到了VAO，不需要再绑定VBO了
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}


GLFWwindow * InitWindowAndFunc() {
    glfwInit();
    // 对GLFW的配置 版本号、次版本号、选择核心模式
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }
    // 前两个参数控制窗口左下角的位置。第三个和第四个参数控制渲染窗口的宽度和高度（像素）
    GL_CALL(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // 光标消失
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 加载材质返回纹理ID
GLuint loadTexture(char const * path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}