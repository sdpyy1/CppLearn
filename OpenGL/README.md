# OpenGL学习
## OpenGL
- 双缓冲: 在每一帧绘制任务完成后，把背后的缓冲区交换到前台，再之前的缓冲区绘制下一帧。
- 错误处理:`glGetError()`检查之前的调用是否出错
- NDC:标准化设备坐标，按比例缩放
- VBO(Vertex Buffer Object):表示了GPU上的一段存储空间对象，在Cpp中表现为unsigned int变量，表示成为GPU端内存对象的一个ID编号
- VAO(Vertex Array Object):顶点数组对象，用于存储一个Mesh网格所有的顶点属性描述信息
## GLFW
## GLAD