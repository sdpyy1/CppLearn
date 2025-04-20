#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <Logger.h>
#include <QFileDialog>
#include <QMessageBox>
#include <qlabel.h>
#include "pushbutton.h"
#include <QVBoxLayout>
#include <QWidget>

class GlobalSetting : public QWidget
{
    Q_OBJECT
public:
    explicit GlobalSetting(QWidget *parent = nullptr);
    void selectSkyBox();
private:
    QVBoxLayout * _mainLayout;
    PushButton * _skyBtn;
    PushButton * _lineModelBtn;
    PushButton * _waitingAdd1;
    PushButton * _waitingAdd2;
    PushButton * _waitingAdd3;
signals:
    void onSettingsChanged(QPair<QString, QString> settings);
    void changeRenderLineFlag();

};

#endif // GLOBALSETTING_H
