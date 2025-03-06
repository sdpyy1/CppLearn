#ifndef WIDGET_H
#define WIDGET_H
#include <QWidget>
#include<QDebug>
#include<QFile>
#include<QFileDialog>
#include<QMessageBox>
#include<QShortcut>
#include"mytextedit.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    // 文件
    QFile file;
    // 重写滚轮事件事件处理
    void wheelEvent(QWheelEvent *event) override;


private slots:
    // 自动生成的槽函数 private slots:表示下边的函数都会被标记为槽函数
    void on_saveButton_clicked();
    // 手动添加一个槽函数
    void onOpenButtonClick();
    void on_closeButton_clicked();
    void on_cursorPosChange();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
