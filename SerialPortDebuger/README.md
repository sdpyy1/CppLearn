# 串口调试助手学习记录
## ui使用
- groupBox
- 栅格布局
- ctrl+拖动控件=copy
- 布局通过设置stretch=1,2,3来控制每个控件的比例

## 模块导入
- 需要去管理模块下载组件
- 需要在cmake或pro文件配置组件，才能include找到

# QString
```cpp
void QStringLearn(){
    QString text;
    // 支持格式化输出 %1 %2 表示占位符
    text = QString("hello%1Ye%2ye").arg("{arg1}").arg("{arg2}");
    // 支持c语言sprintf
    // 字符拼接  支持+=
    text.append("append的字符");
    text.prepend("还支持前拼接");
    // 字数统计
    int count = text.count();
    // 类型转换 toXXX()
    // int i = text.toInt()
    qDebug() << text;
}

```
# QTimer 定时器
- 通过timeout信号，连接槽函数
# QDateTime
- 获取时间
# QThread
- 非ui操作不要占用ui线程
- `worker->moveToThread(&workerThread);`将一个 QObject 及其所有子对象的事件处理转移到指定的线程中

# 其他
当我想点击串口选择的下拉框时，希望刷新系统的串口列表，但是并没有这个信号，所以需要提升为，用自定义控件实现,通过点击事件触发来完成
