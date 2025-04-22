#include "globalsetting.h"


void addBtn(QVBoxLayout * layout, PushButton *& btn, QString title,QWidget * widget);


GlobalSetting::GlobalSetting(QWidget *parent) :
    QWidget(parent)
{
    setMaximumWidth(180);
    // 垂直布局
    _mainLayout = new QVBoxLayout(this);
    _mainLayout->setSpacing(5);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    // 添加按钮
    addBtn(_mainLayout,_skyBtn,"天空盒",this);

    addBtn(_mainLayout,_lineModelBtn,"线框模型",this);
    _lineChange = new Slider(0,100,100,this);
    _lineChange->setValue(50);
    _mainLayout->addWidget(_lineChange);
    _lineChange -> hide();

    addBtn(_mainLayout,_waitingAdd1,"待添加",this);
    addBtn(_mainLayout,_waitingAdd2,"待添加",this);
    addBtn(_mainLayout,_waitingAdd3,"待添加",this);
    _mainLayout->addStretch();



    // 连接信号
    connect(_skyBtn, &PushButton::onClick, this, &GlobalSetting::selectSkyBox);

    connect(_lineModelBtn,&PushButton::onClick,this,[=](){
        if (_selectedObject == nullptr) {
            QMessageBox::warning(this, "警告", "请先选择要绘制的模型");
            return;
        }
        if(_selectedObject != nullptr){
            // 添加一个警告框，内容为请先选择要绘制的模型
            dynamic_cast<QLabel*>(_lineModelBtn->childWidget())->setText(_selectedObject->isRenderLine?"渲染线框":"还原渲染");
            if(!_selectedObject->isRenderLine){
                _lineChange->show();
            }else{
                _lineChange->hide();
            }
        }
        emit changeRenderLineFlag();
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
