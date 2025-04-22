#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <Logger.h>
#include <QFileDialog>
#include <QMessageBox>
#include <qlabel.h>
#include "pushbutton.h"
#include <QVBoxLayout>
#include <QWidget>
#include <Renderable.h>
#include <RoundedCornerWidget.h>
#include <Slider.h>

class GlobalSetting : public QWidget
{
    Q_OBJECT
public:
    explicit GlobalSetting(QWidget *parent = nullptr);
    void selectSkyBox();
    void selectObject(Renderable* object);



private:
    QVBoxLayout * _mainLayout;
    Renderable * _selectedObject;
    // 按钮
    PushButton * _skyBtn;
    PushButton * _lineModelBtn;
    PushButton * _waitingAdd1;
    PushButton * _waitingAdd2;
    PushButton * _waitingAdd3;

    // 操作界面
    Slider * _lineChange;

signals:
    void onSettingsChanged(QPair<QString, QString> settings);
    void changeRenderLineFlag();

};

#endif // GLOBALSETTING_H
