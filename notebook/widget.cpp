#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    // ui绑定widget
    ui->setupUi(this);
    // 让widget窗口变化时内部布局也随着变化
    this->setLayout(ui->verticalLayout);
    // 让页面下的提示框跟着布局变动
    ui->widgetLabelDown->setLayout(ui->horizontalLayout);
    // 此处可进行信号与槽的绑定
    // QObject::connect(ui->openButton,SIGNAL(clicked()),this,SLOT(on_openButton_click()));
    // 用lambda函数方式实现上边一行的功能
    // QObject::connect(ui->openButton,&QPushButton::clicked,this,[=](){
    //     std::cout<<"open"<<std::endl;
    // });
    // 用函数指针实现
    QObject::connect(ui->openButton,&QPushButton::clicked,this,&Widget::on_openButton_click);

}

Widget::~Widget()
{
    delete ui;
}

// 自动生成的槽函数实现位置，🈶函数名约定连接
void Widget::on_savaButton_clicked()
{
    std::cout<<"save"<<std::endl;
}

// 手动添加槽函数(注意手动添加的函数名如果满足上边自动生成的约定，connect后点击会被执行2次)
void Widget::on_openButton_click(){
    std::cout<<"open"<<std::endl;

}

