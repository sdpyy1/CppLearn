# 学习记录
## 信号与槽
当信号被发出，槽中函数会被调用
使用QObject::connect()方法来连接信号和槽
使用方法：
- ui界面编辑
- 代码方式
    - connect（QObejct::connect）
    - lambda
    - 函数指针
    - 自动连接（ui中右键转到槽，选择信号后自动生成代码，由函数名来约定连接的信号和槽）
- 自定义信号和槽
    - 在头文件class的signals：关键字下新建一个方法，并不需要实现
    - slots:关键字下写槽函数
    - connect连接,注意连接时信号函数和槽函数的参数要加上
    - 使用emit xxx来发送某个信号，所有连接的槽函数都会执行
    
## 调试QDebug()
- 头文件 QDebug
- 使用：qDebug()<<"open";
## QFile()
- 读写比较常规（用steam更灵活）
- QFileDialog 文件选择框
- QTextStream 来输入输出文件
## QComboBox下拉框
- 用来让用户自定义文件编码
