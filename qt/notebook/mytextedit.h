#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>
#include<QWheelEvent>
#include<QKeyEvent>
class MyTextEdit : public QTextEdit
{
public:
    MyTextEdit(QWidget * parent);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
private:
    bool isCtrlPress = false;
};

#endif // MYTEXTEDIT_H
