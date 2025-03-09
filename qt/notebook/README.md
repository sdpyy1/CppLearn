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
## QT事件
- 通过重写父类的事件函数，完成事件触发时的处理，注意事件处理完成后accept操作
- 事件过滤器
## 自定义控件
新写一个类来自定义

## 其他
- QString::number(lineNum) 数字转QString
- QList容器（数组，也提供了链表的特性）可以动态增长和缩减，自动管理内存分配。 QList使用写时复制（copy-on-write），copy所有元素时，不进行copy而是共享数据，当尝试修改时，才会进行copy操作
## 记事本完成功能
- 打开文件
- 关闭文件
- 保存新文件
- 设置当前行高亮
- 快捷键
- 字体放大
- 事件机制处理程序关闭
