# OpenGL学习
## OpenGL
- 双缓冲: 在每一帧绘制任务完成后，把背后的缓冲区交换到前台，再之前的缓冲区绘制下一帧。
- 错误处理:`glGetError()`检查之前的调用是否出错
- NDC:标准化设备坐标，按比例缩放
- VBO(Vertex Buffer Object):表示了GPU上的一段存储空间对象，在Cpp中表现为unsigned int变量，表示成为GPU端内存对象的一个ID编号
- VAO(Vertex Array Object):顶点数组对象，用于存储一个Mesh网格所有的顶点属性描述信息
- EBO(Element Buffer Object):存储顶点绘制顺序索引号，这样就可以复用一些顶点（两个三角形有共用边的情况）
  - 描述一个三角形使用了那几个顶点数据的数字序列
# 库
- GLFW
- GLAD
## GLSL语言
为图形计算量身定制的语言
特点：1. 本质是输入转化为输出 2. 程序之间无法通信，只能通过输入和输出进行通信

基本数据类型：
- float
- double
- void
- int
- bool
向量数据类型：(第一个字母表示基本数据类型v:float, d:double,b:bool,i:int,u:unsigned int)
- vecn
- bvecn
- ivecn
- uvecn
- dvecn
向量初始化： vec3 color0=vec3(1.0,0.0,0.0);

重组：vec4 color =vec4(0.1,0.8,0.7,0.6)  新创建一个 vec4 newParam = color.xxyz

Uniform: 当很多个core都需要同一个变量时，使用uniform，只需要存储一次


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

## 插值算法
利用多个顶点对新产生的像素的影响权重，计算新像素的值
三角形的3个顶点都赋予颜色，其中的片元就通过插值算法来设置自己的颜色
- 直线插值：按比例
- 三角形重心插值算法：找重心，连接三个顶点和重心，形成3个三角形，一个顶点的权重就是顶点对面的三角形面积比重