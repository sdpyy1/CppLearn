#include "widget.h"

#include <QApplication>
// QString学习
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
int main(int argc, char *argv[])
{
    // QStringLearn();
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
