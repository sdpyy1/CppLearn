#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<iostream>
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


private slots:
    // 自动生成的槽函数 private slots:表示下边的函数都会被标记为槽函数
    void on_savaButton_clicked();
    // 手动添加一个槽函数
    void on_openButton_click();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
