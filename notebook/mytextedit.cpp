#include "mytextedit.h"

MyTextEdit::MyTextEdit(QWidget *parent) :QTextEdit(parent){

}
// ctrl+滚轮才会触发
void MyTextEdit::wheelEvent(QWheelEvent *event)
{
    if(isCtrlPress == true){
        int wheelChange = event->angleDelta().ry();
        if(wheelChange > 0){
            zoomIn();
        }else{
            zoomOut();
        }
        event->accept(); // 表示事件处理完成，不需要父控件继续处理
    }else{
        QTextEdit::wheelEvent(event); // 不影响父控件对事件的处理(不加这个可能导致滚轮的其他正常功能失效，因为事件在这一层被处理掉了)
    }
}

void MyTextEdit::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control){
        this->isCtrlPress = true;
    }
}

void MyTextEdit::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control){
        this->isCtrlPress = false;
    }
}
