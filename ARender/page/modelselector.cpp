#include <qfiledialog.h>

#include "modelselector.h"

ModelSelector::ModelSelector(QWidget* parent) :
    QWidget(parent)
{
    // Set maximum width
    setMaximumWidth(240);
    
    // Set up the main layout
    _stretchLayout = new QVBoxLayout(this);
    _stretchLayout->setContentsMargins(0, 0, 0, 0);
    _stretchLayout->setSpacing(8);
    setLayout(_stretchLayout);
    // 添加预设model的按钮
    _addTemplateObjectBtn = new PushButton(nullptr, this);
    _addTemplateObjectBtn->setChildWidget(new QLabel("预设模型", _addNewObjectBtn));
    _addTemplateObjectBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _addTemplateObjectBtn->setMargin(32, 24, 32, 24);
    _addTemplateObjectBtn->setRadius(10);
    _addTemplateObjectBtn->setBackgroundColor(QColor(58, 143, 183, 20));
    _stretchLayout->addWidget(_addTemplateObjectBtn);
    _addTemplateObjectBtn->show();
    // Create and add the 'add new' button
    _addNewObjectBtn = new PushButton(nullptr, this);
    _addNewObjectBtn->setChildWidget(new QLabel("添加新模型", _addNewObjectBtn));
    _addNewObjectBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _addNewObjectBtn->setMargin(32, 24, 32, 24);
    _addNewObjectBtn->setRadius(10);
    _addNewObjectBtn->setBackgroundColor(QColor(58, 143, 183, 20));
    _stretchLayout->addWidget(_addNewObjectBtn);
    _addNewObjectBtn->show();
    
    // Create the selection list widget
    _objectList = new ScrollListWidget(this);
    _stretchLayout->addWidget(_objectList);
    _objectList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _objectList->show();

    // Connect
    connect(_addNewObjectBtn, &PushButton::onClick, this, &ModelSelector::addNewObject);
    connect(_addTemplateObjectBtn, &PushButton::onClick, this, &ModelSelector::addTemplateModel);
}

ModelSelector::~ModelSelector() {}

void ModelSelector::addNewObject() {
    QString path = getNewObjectPath();
    if (path.isEmpty()) {
        return;
    }
    Model* model = loadObject(path);
    if (model->status() != Model::LOADED) {
        delete model;
        Logger::error("Invalid model file");
        return;
    }
    ModelSelectable* newSelectable = new ModelSelectable(model, this);
    
    _objectSelectables.push_back(newSelectable);
    _objectList->addWidget(newSelectable);
    
    connect(newSelectable, &ModelSelectable::onSelected, this, [=]() {
        emit onObjectSelected(model);
    });
    connect(newSelectable, &ModelSelectable::onRemoved, this, [=]() {
        removeObject(newSelectable);
    });

}

void ModelSelector::addTemplateModel()
{
    // 预设两个模型
    addNewObjectByPath("./assets/planet/planet.obj");
    addNewObjectByPath("./assets/rock/rock.obj");
}
void ModelSelector::addNewObjectByPath(QString path) {
    if (path.isEmpty()) {
        return;
    }
    Model* model = loadObject(path);
    if (model->status() != Model::LOADED) {
        delete model;
        Logger::error("Invalid model file");
        return;
    }
    ModelSelectable* newSelectable = new ModelSelectable(model, this);
    _objectSelectables.push_back(newSelectable);
    _objectList->addWidget(newSelectable);

    connect(newSelectable, &ModelSelectable::onSelected, this, [=]() {
        emit onObjectSelected(model);
    });
    connect(newSelectable, &ModelSelectable::onRemoved, this, [=]() {
        removeObject(newSelectable);
    });
}

QString ModelSelector::getNewObjectPath() {
    QString path = QFileDialog::getOpenFileName(this, "Select Model", "./assets", "Model Files (*.obj)");
    return path;
}

Model* ModelSelector::loadObject(const QString& path) {
    Model* model = new Model(path.toStdString());
    return model;
}

void ModelSelector::removeObject(ModelSelectable* selectable) {
    // Check if selectable is still in list
    if (!_objectSelectables.contains(selectable)) {
        return;
    }

    // Remove the selectable from the list
    _objectSelectables.removeAll(selectable);

    // Remove the selectable from the list widget
    _objectList->removeWidget(selectable);

    // Delete the selectable
    delete selectable;
}
