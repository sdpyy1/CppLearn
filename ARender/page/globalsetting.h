#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <Logger.h>
#include <QFileDialog>
#include <QMessageBox>
#include <qlabel.h>
#include "modelattrslide.h"
#include "pushbutton.h"
#include <QVBoxLayout>
#include <QWidget>
#include <Renderable.h>
#include <RoundedCornerWidget.h>
#include <Slider.h>
#include <qcombobox.h>

class GlobalSetting : public QWidget
{
    Q_OBJECT
public:

    explicit GlobalSetting(QWidget *parent = nullptr);
    void selectSkyBox();
    void selectObject(Renderable *object);
    void selectTerrain();


private:
    QVBoxLayout * _mainLayout  = nullptr;
    Renderable * _selectedObject = nullptr;
    // 按钮
    PushButton * _skyBtn  = nullptr;
    PushButton * _terrainBtn = nullptr;
    PushButton * _lineModelBtn = nullptr;
    PushButton * _renderPlaneBtn = nullptr;
    PushButton * _addLightBtn = nullptr;
    // 分割线
    QFrame* _line1 = nullptr;
    // 操作界面
    ModelAttributeSlide * _lineChange = nullptr;
    bool _isAddingLight = false;
    ModelAttributeSlide * _lightX = nullptr;
    ModelAttributeSlide * _lightY = nullptr;
    ModelAttributeSlide * _lightZ = nullptr;
    ModelAttributeSlide * _lightColorR = nullptr;
    ModelAttributeSlide * _lightColorG = nullptr;
    ModelAttributeSlide * _lightColorB = nullptr;
    ModelAttributeSlide * _outCutOff = nullptr;
    PushButton * _comfirmAddLightBtn = nullptr;
    QComboBox * _lightTypeComboBox = nullptr;

    void initLightAttr();
    void showLightAttr();
    void hideLightAttr();
signals:
    void onSettingsChanged(QPair<QString, QString> settings);
    void changeRenderLineFlag();
    void changeRenderPlaneFlag();
    void addNewLight(int type,float x,float y, float z,float r,float g,float b,float cutoff);
};

#endif // GLOBALSETTING_H
