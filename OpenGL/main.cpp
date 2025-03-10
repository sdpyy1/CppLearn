#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

GLuint vao,program;


// 定义 GL_CALL 宏
#define GL_CALL(x) \
    do { \
        x; \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            std::cerr << "OpenGL error: " << error << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        } \
    } while (0)

// 响应函数（槽函数）
void frameBufferSizeCallBack(GLFWwindow*window,int width,int height){
    std::cout << "修改viewPoint{" << width << "."<< height << "}";
    glViewport(0,0,width,height);
}
// OpenGL是状态机，可以绑定curVBO，操作时都是操作当前插入的状态
// VBO目的：发送数据到显存
// VAO目的: 将VBO绑定到VAO上，再使用glVertexAttribPointer()来设置属性
void learnVBO(){
    // 构建VBO 每行前三个是位置，后三个是颜色
    float vertices[]={
            -0.5f, -0.5f, 0.0f,1.0f,1.0f,1.0f,
            0.5f, -0.5f, 0.0f,1.0f,1.0f,1.0f,
            0.0f, 0.5f, 0.0f,1.0f,1.0f,1.0f,
            0.3f, 0.3f, 0.3f,1.0f,1.0f,1.0f
    };
    // 创建VBO,glGenBuffers后会设置一个id
    GLuint VBO = 0;
    // n=1表示生成一个（也可以指定多个，VBO对象就需要定义为数组）
    GL_CALL(glGenBuffers(1,&VBO));
    // 销毁一个vbo
    // GL_CALL(glDeleteBuffers(1,&VBO));
    // 绑定当前VBO到openGL的状态上 GL_ARRAY_BUFFER表示当前VBO
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    // 向当前VBO创建数据（使用显存） GL_STATIC_DRAW表示该数据不会进行修改，用于优化效率，也可以选择为GL_DYNAMIC_DRAW，表示数据会频繁修改
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    // 构建VAO
    vao = 0;
    // 生成id
    glGenVertexArrays(1,&vao);
    // 绑定opengl当前状态的vao
    glBindVertexArray(vao);
    // 指定下一次属性设置在vao中的位置
    glEnableVertexAttribArray(0);
    // vao中添加描述信息(index:放在0位置 size:这个属性有几个数字 type:数据类型 normalized:是否归一化 stride:数据的跨度 pointer:内部偏移)  这里就定义了：顶点位置信息是6个float数据的前三个的意思
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    // 指定下一次属性设置在vao中的位置
    glEnableVertexAttribArray(1);
    // 添加颜色属性信息的描述
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));

    // 扫尾工作 解除绑定
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

//shader
void shaderLearn(){
    // glsl源码
    const char* vertexShaderSource = "#version 330 core\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "layout (location = 1) in vec3 aColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_Position = vec4(aPos.x, aPos.y, aPos.z,1.0);\n"
                                     "}\n";
    const char* frameShaderSource = "#version 330 core\n"
                                     "out vec4 FragColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   FragColor = vec4(0.1f,0.8f,0.2f,1.0f);\n"
                                     "}\n";
    // 创建shader程序
    GLuint vertex,fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    fragment = glCreateShader(GL_FRAGMENT_SHADER);

    // 源码输入
    glShaderSource(vertex,1,&vertexShaderSource,NULL);
    glShaderSource(fragment,1,&frameShaderSource,NULL);

    int success = 0;
    char infoLog[1024];
    // 编译源码
    glCompileShader(vertex);
    // 检查
    glGetShaderiv(vertex,GL_COMPILE_STATUS,&success);
    if (!success){
        glGetShaderInfoLog(vertex,1024,NULL,infoLog);
        std::cout << "fail to compile vertex shader" <<infoLog<< std::endl;
    }
    // 编译源码
    glCompileShader(fragment);
    // 检查
    glGetShaderiv(fragment,GL_COMPILE_STATUS,&success);
    if (!success){
        glGetShaderInfoLog(fragment,1024,NULL,infoLog);
        std::cout << "fail to compile fragment shader" <<infoLog<< std::endl;
    }

    // 链接
    // 创建程序壳子
    program = glCreateProgram();
    // 编译结果放入pro
    glAttachShader(program,vertex);
    glAttachShader(program,fragment);
    // 链接操作
    glLinkProgram(program);
    // 检查链接错误
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if (!success){
        glGetProgramInfoLog(program,1024,NULL,infoLog);
        std::cout << "fail to link program" <<infoLog<< std::endl;
    }

    // 扫尾工作
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// 渲染
void render(){
    // 画布清理
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
    // 绑定program
    glUseProgram(program);
    // 绑定vao
    glBindVertexArray(vao);
    // 绘制
    glDrawArrays(GL_TRIANGLES,0,3);
    // 解绑
    glBindVertexArray(0);
    glUseProgram(0);
}

int main()
{
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建一个窗口对象
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
    // 设置当前窗口为绘制台
    glfwMakeContextCurrent(window);
    // 设置监听(类似qt的connect)
    glfwSetFramebufferSizeCallback(window,frameBufferSizeCallBack); // 绑定监听处理函数

    // 使用glad加载opengl函数的具体实现
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "fail to init glad";
        return -1;
    }
    // shader程序
    shaderLearn();
    // viewport 视口，表示渲染的范围
    glViewport(0,0,800,600);
    // 设置清理颜色
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    // 创建VBO和VAO
    learnVBO();
    // 窗口循环
    while(!glfwWindowShouldClose(window)){

        // 接收并分发窗口消息
        glfwPollEvents();
        // 渲染
        render();
        // 更换双缓存
        glfwSwapBuffers(window);
    }
    return 0;
}