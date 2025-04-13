//
// Created by 刘卓昊 on 2025/4/3.
//

#include <iostream>
#include "Shader.h"
#include "camera.h"
#include "macros.h"
#include "model.h"
GLuint loadCubemap(vector<std::string> faces);
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


    // 着色器设置
    Shader bagShader("./shader/bag.vert", "./shader/bag.frag");
    // 模型导入
    Model model = Model("./assets/backpack/backpack.obj");



    vector<std::string> faces
            {
                    "right.jpg",
                    "left.jpg",
                    "top.jpg",
                    "bottom.jpg",
                    "front.jpg",
                    "back.jpg"
            };
    GLuint cubemapTexture = loadCubemap(faces);




    while (!glfwWindowShouldClose(window))
    {

        // 清理窗口
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

        // 这里开始都是在FBO上渲染
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // 绘制物体
        drawModel(bagShader, model,{1.f,0.f,-3.f},{1.0f,1.0f,1.0f});


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
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}