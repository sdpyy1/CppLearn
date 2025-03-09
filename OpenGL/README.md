# OpenGL学习
## OpenGL
- 双缓冲: 在每一帧绘制任务完成后，把背后的缓冲区交换到前台，再之前的缓冲区绘制下一帧。
- 错误处理:`glGetError()`检查之前的调用是否出错
- NDC:标准化设备坐标，按比例缩放
- VBO(Vertex Buffer Object):表示了GPU上的一段存储空间对象，在Cpp中表现为unsigned int变量，表示成为GPU端内存对象的一个ID编号
- VAO(Vertex Array Object):顶点数组对象，用于存储一个Mesh网格所有的顶点属性描述信息
# 库
- GLFW
- GLAD

## Shader
`对三角形数据的处理，分为顶点处理和片元处理`
- VertexShader
  layout(location = 0) in vec3 aPos; 
    - in：输入 
    - vec3:三维向量 
    - aPos:变量名  
    - 表示输入的三维变量是aPos 
    - layout(location = 0)：表示VertexShader去VAO的第n个属性描述中去取数据
  gl_Position = vec4(aPosx,aPosy,aPosz,1.0)
    - gl_Position：glsl内置变量
    - 负责后续阶段输出顶点位置处理的结果
- FragmentShader
    当顶点处理完成后，会定位需要处理的片元，每个片元进入一个core，执行FragmentShader
- GLSL语言（Graphic Library Shader Language 类c语言，为图形学计算量身定制） 将输入转化为输出的程序
  - 需要在代码中写源码，编译，链接