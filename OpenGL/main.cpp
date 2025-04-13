//
// Created by 刘卓昊 on 2025/4/3.
//

#include <iostream>
#include "Shader.h"
#include "camera.h"
#include "macros.h"
#include "model.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
GLFWwindow * InitWindowAndFunc();
GLuint loadTexture(char const * path);
void drawModel(Shader &bagShader, Model &model, glm::vec3 translate, glm::vec3 scale) ;// view/projection transformations

void processFrameTimeForMove();

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
    stbi_set_flip_vertically_on_load(true);
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    // 启动面剔除
    glEnable(GL_CULL_FACE);
    // 启动混合
    glEnable(GL_BLEND);
    // 设置F
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 创建帧缓冲对象
    GLuint FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // 创建一个纹理附件
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // 创建一个空的纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    // 渲染缓冲对象RBO
    GLuint RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
    // 检查帧缓冲是否完整
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // 着色器设置
    Shader bagShader("./shader/bag.vert", "./shader/bag.frag");
    // 模型导入
    Model model = Model("./assets/backpack/backpack.obj");


    // 临时设置一个玻璃
    float cubeVertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            // Left face
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
            // Right face
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            // Bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
            // Top face
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left
    };
    GLuint grassVBO, grassVAO;
    glGenBuffers(1, &grassVBO);
    glGenVertexArrays(1, &grassVAO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    GLuint grassTexture = loadTexture("./assets/glass.png");

    // 用于离线渲染的正方形
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f,0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f,0.0f, 0.0f,
            1.0f, -1.0f,  0.0f,1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f,0.0f, 1.0f,
            1.0f, -1.0f,  0.0f,1.0f, 0.0f,
            1.0f,  1.0f,  0.0f,1.0f, 1.0f
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);



    while (!glfwWindowShouldClose(window))
    {

        // 清理窗口
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        // 离线渲染
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        // 这里开始都是在FBO上渲染
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // 绘制物体
        drawModel(bagShader, model,{1.f,0.f,-3.f},{1.0f,1.0f,1.0f});
        // 绘制草
        glActiveTexture(GL_TEXTURE0);
        bagShader.setInt("texture_diffuse1",0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glBindVertexArray(grassVAO);
        glm::mat4 modelTrans = glm::mat4(1.0f);
        modelTrans = glm::translate(modelTrans, {0,0.5,-1});
        modelTrans = glm::scale(modelTrans, {1,1,1});
        bagShader.setMat4("model", modelTrans);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 回到0号帧缓冲
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(quadVAO);
        bagShader.setInt("texture_diffuse1",0);
        glBindTexture(GL_TEXTURE_2D, texture);
        bagShader.use();
        bagShader.setMat4("model", glm::mat4(1.0f));
        bagShader.setMat4("view", glm::mat4(1.0f));
        bagShader.setMat4("projection", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 事件处理
        glfwPollEvents();
        // 双缓冲
        glfwSwapBuffers(window);
        processFrameTimeForMove();
        processInput(window);

    }
    glfwTerminate();

    return 0;
}

void processFrameTimeForMove() {
    auto currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void drawModel(Shader &bagShader, Model &model, glm::vec3 translate, glm::vec3 scale) {
    bagShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    bagShader.setMat4("projection", projection);
    bagShader.setMat4("view", view);
    // render the loaded model
    glm::mat4 modelTrans = glm::mat4(1.0f);
    modelTrans = glm::translate(modelTrans, translate); // translate it down so it's at the center of the scene
    modelTrans = glm::scale(modelTrans, scale);    // it's a bit too big for our scene, so scale it down
    bagShader.setMat4("model", modelTrans);
    model.Draw(bagShader);
};

GLFWwindow * InitWindowAndFunc() {
    glfwInit();
    // 对GLFW的配置 版本号、次版本号、选择核心模式
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
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
    GL_CALL(glfwSetFramebufferSizeCallback(window, framebuffer_size_callback));
    GL_CALL(glfwSetCursorPosCallback(window, mouse_callback));
    GL_CALL(glfwSetScrollCallback(window, scroll_callback));
    // 光标消失
    GL_CALL(glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED));
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
    stbi_set_flip_vertically_on_load(false);
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

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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