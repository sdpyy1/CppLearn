#include "globalsetting.h"

#include <QComboBox>


void addBtn(QVBoxLayout * layout, PushButton *& btn, QString title,QWidget * widget);
void addLine(QVBoxLayout * layout, QFrame*& line);



GlobalSetting::GlobalSetting(QWidget *parent) : QWidget(parent) {
    setMaximumWidth(250);
    // 垂直布局
    _mainLayout = new QVBoxLayout(this);
    _mainLayout->setSpacing(5);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    // 添加按钮
    addBtn(_mainLayout, _skyBtn, "天空盒", this);
    addBtn(_mainLayout, _terrainBtn, "地形", this);
    addBtn(_mainLayout, _renderPlaneBtn, "渲染地板", this);
    addLine(_mainLayout, _line1);

    addBtn(_mainLayout, _lineModelBtn, "线框模型", this);
    _lineChange = new ModelAttributeSlide("模型细分", 0.1, 10, 100, this);
    _lineChange->setValue(50);
    _mainLayout->addWidget(_lineChange);
    _lineChange->hide();

    addBtn(_mainLayout, _addLightBtn, "添加光照", this);
    initLightAttr();

    _mainLayout->addStretch();

    // 连接信号
    connect(_skyBtn, &PushButton::onClick, this, &GlobalSetting::selectSkyBox);
    connect(_terrainBtn, &PushButton::onClick, this,
            &GlobalSetting::selectTerrain);
    connect(_renderPlaneBtn, &PushButton::onClick, this,
            [=]() { emit changeRenderPlaneFlag(); });

    connect(_lineModelBtn, &PushButton::onClick, this, [=]() {
        if (_selectedObject != nullptr) {
            dynamic_cast<QLabel *>(_lineModelBtn->childWidget())
                ->setText(_selectedObject->isRenderLine ? "渲染线框" : "还原渲染");
            if (!_selectedObject->isRenderLine) {
                _lineChange->show();
            } else {
                _lineChange->hide();
            }
        }
        emit changeRenderLineFlag();
    });

    connect(_addLightBtn, &PushButton::onClick, this, [=]() {
        if (_isAddingLight) {
            _isAddingLight = false;
            hideLightAttr();
        } else {
            _isAddingLight = true;
            showLightAttr();
        }
    });

    connect(_comfirmAddLightBtn, &PushButton::onClick,this,[=](){
        emit addNewLight(_lightTypeComboBox->currentIndex(),_lightX->val(),_lightY->val(),_lightZ->val(),_lightColorR->val(),_lightColorG->val(),_lightColorB->val(),_outCutOff->val());
    });
}

void GlobalSetting::selectSkyBox() {
    // Select a directory that contains back.jpg, bottom.jpg, front.jpg, left.jpg, right.jpg, top.jpg
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Skybox Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        Logger::warning("Tempt to open invalid skybox folder");
        emit onSettingsChanged(QPair<QString, QString>("skybox", ""));
        return;
    }

    // Check if the directory contains all the required files
    QDir skyboxDir(dir);
    QStringList skyboxFiles = skyboxDir.entryList(QStringList() << "*.jpg", QDir::Files);
    if (
        skyboxFiles.indexOf("back.jpg") == -1   ||
        skyboxFiles.indexOf("bottom.jpg") == -1 ||
        skyboxFiles.indexOf("front.jpg") == -1  ||
        skyboxFiles.indexOf("left.jpg") == -1   ||
        skyboxFiles.indexOf("right.jpg") == -1  ||
        skyboxFiles.indexOf("top.jpg") == -1
        ) {
        Logger::warning("天空盒文件夹错误，情况天空盒设置");
        emit onSettingsChanged(QPair<QString, QString>("skybox", ""));
        return;
    }
    emit onSettingsChanged(QPair<QString, QString>("skybox", dir));
}

void GlobalSetting::selectTerrain() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Terrain Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        Logger::warning("Tempt to open invalid terrain folder");
        emit onSettingsChanged(QPair<QString, QString>("terrain", ""));
        return;
    }

    QDir terrainDir(dir);
    // filter *.jpg and *.png
    QStringList terrainFiles = terrainDir.entryList(QStringList() << "*.jpg" << "*.png", QDir::Files);
    if (
        terrainFiles.indexOf("heightmap.png") == -1 ||
        terrainFiles.indexOf("texture.jpg") == -1
        ) {
        QMessageBox::warning(this, "Error", "The selected directory does not contain all the required files.");
        Logger::warning("Tempt to open invalid terrain folder");
        emit onSettingsChanged(QPair<QString, QString>("terrain", ""));
        return;
    }

    emit onSettingsChanged(QPair<QString, QString>("terrain", dir));
}

void GlobalSetting::selectObject(Renderable *object)
{
    _selectedObject = object;
    // 空指针处理，否则会崩溃
    if(_selectedObject == nullptr){
        _lineChange->hide();
        return;
    }
    dynamic_cast<QLabel*>(_lineModelBtn->childWidget())->setText(_selectedObject->isRenderLine?"还原渲染":"渲染线框");
    if(_selectedObject->isRenderLine){
        _lineChange->show();
    }else{
        _lineChange->hide();
    }
}


void addBtn(QVBoxLayout * layout, PushButton *& btn, QString title,QWidget * widget){
    btn = new PushButton(nullptr, widget);
    btn->setChildWidget(new QLabel(title, btn));
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setMargin(32, 24, 32, 24);
    btn->setRadius(10);
    btn->setBackgroundColor(QColor(58, 143, 183, 20));
    layout->addWidget(btn);
    btn->show();
}
void addLine(QVBoxLayout * layout, QFrame*& line){
    line = new QFrame();
    line->setStyleSheet("background-color: rgba(58,143,183,50);");  // 与按钮背景色协调
    line->setFixedHeight(1);  // 设置1像素高度
    layout->addWidget(line);
}

void GlobalSetting::initLightAttr() {

    _lightTypeComboBox = new QComboBox(this);
    _lightTypeComboBox->addItem("点光源");
    _lightTypeComboBox->addItem("平行光");
    _lightTypeComboBox->addItem("聚光");
    _lightTypeComboBox->setCurrentIndex(0);
    _mainLayout->addWidget(_lightTypeComboBox);
    _lightTypeComboBox->hide();
    _lightX = new ModelAttributeSlide("X", -10, 10, 10, this);
    _lightY = new ModelAttributeSlide("Y", -10, 10, 10, this);
    _lightZ = new ModelAttributeSlide("Z", -10, 10, 10, this);
    _lightColorR = new ModelAttributeSlide("R", 0, 255, 255, this);
    _lightColorG = new ModelAttributeSlide("G", 0, 255, 255, this);
    _lightColorB = new ModelAttributeSlide("B", 0, 255, 255, this);
    _lightColorR->setValue(255);
    _lightColorG->setValue(255);
    _lightColorB->setValue(255);
    _outCutOff = new ModelAttributeSlide("cutoff", 0, 180, 180, this);
    _mainLayout->addWidget(_lightX);
    _mainLayout->addWidget(_lightY);
    _mainLayout->addWidget(_lightZ);
    _mainLayout->addWidget(_lightColorR);
    _mainLayout->addWidget(_lightColorG);
    _mainLayout->addWidget(_lightColorB);
    _mainLayout->addWidget(_outCutOff);
    _lightX->hide();
    _lightY->hide();
    _lightZ->hide();
    _lightColorR->hide();
    _lightColorG->hide();
    _lightColorB->hide();
    _outCutOff->hide();
    _comfirmAddLightBtn = new PushButton(nullptr, this);
    _comfirmAddLightBtn->setChildWidget(new QLabel("确认添加", _comfirmAddLightBtn));
    _comfirmAddLightBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _comfirmAddLightBtn->setMargin(10, 10, 10, 10);
    _comfirmAddLightBtn->setRadius(30);
    _comfirmAddLightBtn->setBackgroundColor(QColor(144, 238, 144, 60));
    _mainLayout->addWidget(_comfirmAddLightBtn);
    _comfirmAddLightBtn->hide();
}

void GlobalSetting::showLightAttr()
{
    _lightX->show();
    _lightY->show();
    _lightZ->show();
    _lightColorR->show();
    _lightColorG->show();
    _lightColorB->show();
    _comfirmAddLightBtn->show();
    _lightTypeComboBox->show();
    _outCutOff->show();
}

void GlobalSetting::hideLightAttr()
{
    _lightX->hide();
    _lightY->hide();
    _lightZ->hide();
    _lightColorR->hide();
    _lightColorG->hide();
    _lightColorB->hide();
    _comfirmAddLightBtn->hide();
    _lightTypeComboBox->hide();
    _outCutOff->hide();

}
