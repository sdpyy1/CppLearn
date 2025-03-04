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
    QObject::connect(ui->openButton,SIGNAL(clicked()),this,SLOT(on_openButton_click()));
    // 用lambda函数方式实现上边一行的功能
    // QObject::connect(ui->openButton,&QPushButton::clicked,this,[=](){
    //     std::cout<<"open"<<std::endl;
    // });
    // 用函数指针实现
    // QObject::connect(ui->openButton,&QPushButton::clicked,this,&Widget::on_openButton_click);

    // 编码下拉框选择后，触发槽函数
    QObject::connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(onCurrentIndexChanged(int)));

}

Widget::~Widget()
{
    delete ui;
}

// 自动生成的槽函数实现位置，🈶函数名约定连接
void Widget::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("save file"),"C:/Users/Administrator/Desktop",tr("Text Files (*.txt)"));
    file.setFileName(fileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        qDebug() << "file open error";
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    QString content = ui->textEdit->toPlainText();
    out << content;
}

// 手动添加槽函数(注意手动添加的函数名如果满足上边自动生成的约定，connect后点击会被执行2次)
// 打开文件按钮
void Widget::on_openButton_click(){
    QString fileName = QFileDialog::getOpenFileName(this,tr("标题，tr用于国际化"),"C:/Users/Administrator/Desktop",tr("Text Files (*.txt)"));
    qDebug() << "open file :"<<fileName;
    ui->textEdit->clear();
    file.setFileName(fileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug()<<"open file error!";
        return;
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    while(!in.atEnd()){
        QString context = in.readLine();
        qDebug() << "file content: "<<context;
        ui->textEdit->append(context);
    }
}


void Widget::on_closeButton_clicked()
{
    if(file.isOpen()){
        file.close();
        ui->textEdit->clear();
    }
}

void Widget::onCurrentIndexChanged(int index)
{
    // qt6 QStringConverter的编码被限制，继续研究文件乱码问题无价值，不开发此功能，默认全部使用utf-8
    qDebug() << index;
}

