#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint vao, program, texture;

#define GL_CALL(x) \
    do { \
        x; \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            std::cerr << "OpenGL error: " << error << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        } \
    } while (0)

// 响应函数（槽函数）
void frameBufferSizeCallBack(GLFWwindow* window, int width, int height) {
    std::cout << "修改viewPoint{" << width << "." << height << "}";
    GL_CALL(glViewport(0, 0, width, height));
}
// 定义 GL_CALL 宏
// 初始化窗口
GLFWwindow *initWindow() {// 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建一个窗口对象
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
    // 设置当前窗口为绘制台
    glfwMakeContextCurrent(window);
    // 设置监听(类似qt的connect)
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallBack); // 绑定监听处理函数

    return window;
}

// OpenGL是状态机，可以绑定curVBO，操作时都是操作当前插入的状态
// VBO目的：发送数据到显存
// VAO目的: 将VBO绑定到VAO上，再使用glVertexAttribPointer()来设置属性
void learnVBO() {
    // 构建VBO 每行前三个是位置，后三个是颜色
    float vertices[] = {
            -0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 1.0f, 0.0f, 0.0f,// 3维坐标 + 3维颜色 + 2维uv坐标
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  0.5f, 0.5f,
            0.3f, 0.8f, 0.2f, 0.0f, 1.0f, 0.0f,  0.2f, 0.5f,
    };

    unsigned int indices[] = {
            0, 1, 2,
            2, 1, 3
    };
    // 创建VBO,glGenBuffers后会设置一个id
    GLuint VBO = 0;
    // n=1表示生成一个（也可以指定多个，VBO对象就需要定义为数组）
    GL_CALL(glGenBuffers(1, &VBO));
    // 销毁一个vbo
    // GL_CALL(glDeleteBuffers(1,&VBO));
    // 绑定当前VBO到openGL的状态上 GL_ARRAY_BUFFER表示当前VBO
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    // 向当前VBO创建数据（使用显存） GL_STATIC_DRAW表示该数据不会进行修改，用于优化效率，也可以选择为GL_DYNAMIC_DRAW，表示数据会频繁修改
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // 构建VAO
    vao = 0;
    // 生成id
    GL_CALL(glGenVertexArrays(1, &vao));

    // 根据glsl的参数输入顺序来获取vao中数据定义的顺序
    GLint posAttrib = glGetAttribLocation(program, "aPos");
    GLint colorAttrib = glGetAttribLocation(program, "aColor");
    GLint  uvAttrib = glGetAttribLocation(program, "aUV");
    // 绑定opengl当前状态的vao
    GL_CALL(glBindVertexArray(vao));
    // 指定下一次属性设置在vao中的位置
    GL_CALL(glEnableVertexAttribArray(posAttrib));
    // vao中添加描述信息(index:放在0位置 size:这个属性有几个数字 type:数据类型 normalized:是否归一化 stride:数据的跨度 pointer:内部偏移)  这里就定义了：顶点位置信息是6个float数据的前三个的意思
    GL_CALL(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
//    // 指定下一次属性设置在vao中的位置  color已经用纹理替换
//    GL_CALL(glEnableVertexAttribArray(colorAttrib));
//    // 添加颜色属性信息的描述
//    GL_CALL(glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))));
    // 指定下一次属性设置在vao中的位置
    GL_CALL(glEnableVertexAttribArray(uvAttrib));
    // 添加uv坐标属性信息的描述
    GL_CALL(glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))));

    // 创建EBO(与VBO类似，只是GL_ELEMENT_ARRAY_BUFFER这个参数不一样)
    GLuint ebo = 0;
    GL_CALL(glGenBuffers(1, &ebo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
    // 绑定EBO到VAO
    GL_CALL(glBindVertexArray(vao));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));

    // 扫尾工作 解除绑定
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

//shader glsl编译和连接
void learnShader() {
    // glsl源码
    // vertexShader源码：输入坐标和颜色，并直接输出（没做处理）
    const char* vertexShaderSource = "#version 330 core\n"
                                     // "layout (location = 0) in vec3 aPos;\n"   // 输入位置1的3维坐标
                                     "in vec3 aPos;\n"   // 输入位置1的3维坐标(不指定位置版本，就按从头开始)
                                     // "layout (location = 1) in vec3 aColor;\n" // 输入位置2的颜色数据
//                                     "in vec3 aColor;\n" // 输入位置2的颜色数据(不指定位置版本，就是从上一个变量输入完成之后的3个数据)‘
                                     "in vec2 aUV;\n"
//                                     "out vec3 color;\n"
                                     "out vec2 uv;\n"
                                     "uniform float time;\n"
                                     "void main()\n"
                                     "{\n"
//                                     "   gl_Position = vec4(aPos.x * (sin(time)+1.0) / 2.0, aPos.y, aPos.z,1.0);\n"  // 最后的1.0是为了表示齐次坐标 这段代码会让三角形旋转
                                     "   gl_Position = vec4(aPos,1.0);\n"  // 最后的1.0是为了表示齐次坐标
//                                     "   color = aColor * (sin(time)+1.0) / 2.0;\n"            // 让color随时间变动
                                     "   uv = vec2(aUV.x + time*0.3,aUV.y);\n"
                                     "}\n";
    // frameShader源码
    const char* frameShaderSource = "#version 330 core\n"
                                    "out vec4 FragColor;\n"   // 表示最后要输出的变量名
                                    "uniform sampler2D sampler;\n"
                                    "in vec3 color;\n"  // 输入是在vertex中定义的输出color，通过插值之后获得的颜色
                                    "in vec2 uv;\n"
                                    "void main()\n"
                                    "{\n" // 片元的颜色来自采样器，采样位置来自uv坐标，uv坐标来自顶点设置的uv坐标
                                    "   FragColor = texture(sampler,uv);\n"
                                    "}\n";
    // 创建shader程序
    GLuint vertex, fragment;
    GL_CALL(vertex = glCreateShader(GL_VERTEX_SHADER));
    GL_CALL(fragment = glCreateShader(GL_FRAGMENT_SHADER));

    // 源码输入
    GL_CALL(glShaderSource(vertex, 1, &vertexShaderSource, NULL));
    GL_CALL(glShaderSource(fragment, 1, &frameShaderSource, NULL));

    int success = 0;
    char infoLog[1024];
    // 编译源码
    GL_CALL(glCompileShader(vertex));
    // 检查
    GL_CALL(glGetShaderiv(vertex, GL_COMPILE_STATUS, &success));
    if (!success) {
        GL_CALL(glGetShaderInfoLog(vertex, 1024, NULL, infoLog));
        std::cout << "fail to compile vertex shader" << infoLog << std::endl;
    }
    // 编译源码
    GL_CALL(glCompileShader(fragment));
    // 检查
    GL_CALL(glGetShaderiv(fragment, GL_COMPILE_STATUS, &success));
    if (!success) {
        GL_CALL(glGetShaderInfoLog(fragment, 1024, NULL, infoLog));
        std::cout << "fail to compile fragment shader" << infoLog << std::endl;
    }

    // 链接
    // 创建程序壳子
    GL_CALL(program = glCreateProgram());
    // 编译结果放入pro
    GL_CALL(glAttachShader(program, vertex));
    GL_CALL(glAttachShader(program, fragment));
    // 链接操作
    GL_CALL(glLinkProgram(program));
    // 检查链接错误
    GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &success));
    if (!success) {
        GL_CALL(glGetProgramInfoLog(program, 1024, NULL, infoLog));
        std::cout << "fail to link program" << infoLog << std::endl;
    }

    // 扫尾工作
    GL_CALL(glDeleteShader(vertex));
    GL_CALL(glDeleteShader(fragment));
}

// 纹理
void learnTexture() {
    int width, height, channels;
    // 设置y轴反转，因为opengl的坐标系和图片的坐标系不一样，所以需要反转y轴（y轴反向）
    stbi_set_flip_vertically_on_load(true);
    // 读取图片 &参数会赋值给width,height,channels 通过图片获得
    unsigned char* imgData = stbi_load("image.jpeg",&width,&height,&channels,STBI_rgb_alpha);
    // 生成纹理
    glGenTextures(1, &texture);
    // 激活纹理单元(纹理单元是内置的，只需要告诉激活的编号)
    glActiveTexture(GL_TEXTURE0);
    // 绑定纹理对象(0号纹理单元上绑定了刚才生成的纹理（此时的纹理还是空的）)
    glBindTexture(GL_TEXTURE_2D, texture);
    // 传输纹理数据(正式开辟显存)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
    // 设置纹理参数 GL_TEXTURE_MIN_FILTER:表示要设置的参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置wrapper GL_TEXTURE_WRAP_S和GL_TEXTURE_WRAP_T 表示uv两个方向可以单独设置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // 释放数据 已经发到GPU了，可以释放了
    stbi_image_free(imgData);
}


// 每一帧的渲染流程
void render() {
    // 画布清理
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
    // 绑定program
    GL_CALL(glUseProgram(program));
    // uniform变量设置  设置时间变量，并且每一帧都设置，这样才能让time变量变化
    GLint location = glGetUniformLocation(program, "time");
    GL_CALL(glUniform1f(location, glfwGetTime()));
    GLint sampler = glGetUniformLocation(program, "sampler");
    GL_CALL(glUniform1i(sampler, 0));
    // 绑定vao
    GL_CALL(glBindVertexArray(vao));
    // 绘制
    // glDrawArrays(GL_TRIANGLES,0,4); // 有EBO就不能用这个函数了    indices:EBO内部偏移量，也可以在这里输入数组表示EBO，但效率低
    GL_CALL(glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0));
    // 解绑
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glUseProgram(0));
}


int main() {
    // 初始化窗口
    GLFWwindow *window = initWindow();
    // 使用glad加载opengl函数的具体实现
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "fail to init glad";
        return -1;
    }
    // viewport 视口，表示渲染的范围
    GL_CALL(glViewport(0, 0, 800, 600));
    // 设置清理颜色
    GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
    // shader程序
    learnShader();
    // 创建VBO和VAO
    learnVBO();
    // 纹理
    learnTexture();
    // 窗口循环
    while (!glfwWindowShouldClose(window)) {
        // 接收并分发窗口消息
        GL_CALL(glfwPollEvents());
        // 渲染
        render();
        // 更换双缓存
        glfwSwapBuffers(window);
    }
    return 0;
}