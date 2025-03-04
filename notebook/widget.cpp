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
}

Widget::~Widget()
{
    delete ui;
}
