#include "globalsetting.h"

void addBtn(QVBoxLayout * layout, PushButton *& btn, QString title,QWidget * widget);


GlobalSetting::GlobalSetting(QWidget *parent) :
    QWidget(parent)
{
    setMaximumWidth(180);
    // 垂直布局
    _mainLayout = new QVBoxLayout(this);

    // 添加按钮
    addBtn(_mainLayout,_skyBtn,"天空盒",this);
    addBtn(_mainLayout,_lineModelBtn,"线框模型",this);
    addBtn(_mainLayout,_waitingAdd1,"待添加",this);
    addBtn(_mainLayout,_waitingAdd2,"待添加",this);
    addBtn(_mainLayout,_waitingAdd3,"待添加",this);

    // 连接信号
    connect(_skyBtn, &PushButton::onClick, this, &GlobalSetting::selectSkyBox);

    connect(_lineModelBtn,&PushButton::onClick,this,[=](){
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
