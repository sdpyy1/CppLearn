#include "mycombobox.h"

MyComboBox::MyComboBox(QWidget *parent) :QComboBox(parent)
{

}

void MyComboBox::mousePressEvent(QMouseEvent *event)
{
    // 获取当前存在的串口 因为这里拿不到ui资源
    // QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    // for(QSerialPortInfo info: list){
    //     ui->comboBoxSerialName->addItem(info.portName());
    // }
    // 因为拿不到ui资源，所以选择发送一个信号 再widget中接收信号，在widget中写槽函数！
    emit fresh();
    // 必须继续让原控件正常功能运行
    QComboBox::mousePressEvent(event);
}
