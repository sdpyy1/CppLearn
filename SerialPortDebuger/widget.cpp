#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    // this->setLayout(ui->gridLayout_8);
}

Widget::~Widget()
{
    delete ui;
}
