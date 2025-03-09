#include "widget.h"
#include "./ui_widget.h"

#include <QWheelEvent>

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
    QObject::connect(ui->openButton,SIGNAL(clicked()),this,SLOT(onOpenButtonClick()));
    // 用lambda函数方式实现上边一行的功能
    // QObject::connect(ui->openButton,&QPushButton::clicked,this,[=](){
    //     std::cout<<"open"<<std::endl;
    // });
    // 用函数指针实现
    // QObject::connect(ui->openButton,&QPushButton::clicked,this,&Widget::on_openButton_click);

    // 编码下拉框选择后，触发槽函数
    // QObject::connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(onCurrentIndexChanged(int)));

    // 绑定光标位置改变事件
    QObject::connect(ui->myTextEdit,SIGNAL(cursorPositionChanged()),this,SLOT(on_cursorPosChange()));

    // 快捷键绑定
    QShortcut * saveShortcut = new QShortcut(QKeySequence(tr("Ctrl+s","File|Open")),this);
    QObject::connect(saveShortcut,&QShortcut::activated,this,[=](){
        on_saveButton_clicked();
    });
    // 放大快捷键
    QShortcut * zoomInShortcut = new QShortcut(QKeySequence(tr("Ctrl+shift+=","File|Open")),this);
    QObject::connect(zoomInShortcut,&QShortcut::activated,this,[=](){
        QFont font = ui->myTextEdit->font();
        int fontSize = font.pointSizeF();
        if(fontSize == -1) return;
        int newFontSize = fontSize + 1;
        font.setPointSize(newFontSize);
        ui->myTextEdit->setFont(font);
    });
    QShortcut * zoomOutShortcut = new QShortcut(QKeySequence(tr("Ctrl+shift+-","File|Open")),this);
    QObject::connect(zoomOutShortcut,&QShortcut::activated,this,[=](){
        QFont font = ui->myTextEdit->font();
        int fontSize = font.pointSizeF();
        if(fontSize == -1) return;
        int newFontSize = fontSize -1;
        font.setPointSize(newFontSize);
        ui->myTextEdit->setFont(font);
    });

    // 事件过滤器安装（事件触发后首先进行的就是事件过滤器，然后才会分发） 这里this表示调用谁的过滤器，因为是在Widget类中实现的过滤器，所以就该他来调用
    ui->myTextEdit->installEventFilter(this);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::wheelEvent(QWheelEvent *event)
{
    // qDebug() << event->angleDelta();
}

void Widget::closeEvent(QCloseEvent *event)
{
    QMessageBox q;
    q.setText("关闭事件触发啦");
    q.exec();
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Type::KeyPress){
        qDebug() << "按下事件被过滤器过滤";
    }
    return false; // 表示继续执行事件，true表示停止
}


// 自动生成的槽函数实现位置，🈶函数名约定连接
void Widget::on_saveButton_clicked()
{
    if(!file.isOpen()){
        QString fileName = QFileDialog::getSaveFileName(this,tr("save file"),"C:/Users/Administrator/Desktop",tr("Text Files (*.txt)"));
        file.setFileName(fileName);
        if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){
            qDebug() << "file open error";
        }
        this->setWindowTitle(fileName);
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    QString content = ui->myTextEdit->toPlainText();
    out << content;
}

// 手动添加槽函数(注意手动添加的函数名如果满足上边自动生成的约定，connect后点击会被执行2次)
// 打开文件按钮
void Widget::onOpenButtonClick(){
    QString fileName = QFileDialog::getOpenFileName(this,tr("标题，tr用于国际化"),"C:/Users/Administrator/Desktop",tr("Text Files (*.txt)"));
    qDebug() << "open file :"<<fileName;
    ui->myTextEdit->clear();
    file.setFileName(fileName);
    if(!file.open(QIODevice::ReadWrite|QIODevice::Text)){
        qDebug()<<"open file error!";
        return;
    }
    this->setWindowTitle(fileName);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    while(!in.atEnd()){
        QString context = in.readLine();
        qDebug() << "file content: "<<context;
        ui->myTextEdit->append(context);
    }
}


void Widget::on_closeButton_clicked()
{
    QMessageBox msgBox;
    int ret = QMessageBox::warning(this, tr("notebook"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard
                                       | QMessageBox::Cancel,
                                   QMessageBox::Save);
    switch (ret) {
    case QMessageBox::Save:
        // Save was clicked
        on_saveButton_clicked();
        break;
    case QMessageBox::Discard:
        // Don't Save was clicked
        ui->myTextEdit->clear();
        if(file.isOpen()){
            file.close();
            this->setWindowTitle("记事本");
        }
        break;
    case QMessageBox::Cancel:
        return;
    default:
        // should never be reached
        break;
    }

}

void Widget::on_cursorPosChange()
{

    QTextCursor cursor = ui->myTextEdit->textCursor();
    // 列号
    int columnNum = cursor.columnNumber() + 1;
    // 行号
    int lineNum = cursor.blockNumber() + 1;
    ui->posLabel->setText("第" + QString::number(lineNum) +"行第"+QString::number(columnNum) + "列");
    // 设置当前行高亮
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection ext;
    ext.cursor=ui->myTextEdit->textCursor();
    QBrush qBrush(Qt::yellow);
    ext.format.setBackground(qBrush);
    ext.format.setProperty(QTextFormat::FullWidthSelection,true);
    extraSelections.append(ext);
    ui->myTextEdit->setExtraSelections(extraSelections);
}

// void Widget::onCurrentIndexChanged(int index)
// {
//     // qt6 QStringConverter的编码被限制，继续研究文件乱码问题无价值，不开发此功能，默认全部使用utf-8
//     qDebug() << index;
// }

