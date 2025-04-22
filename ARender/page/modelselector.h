#pragma once

#include <qwidget.h>
#include <qboxlayout.h>
#include <qlist.h>

#include "scrolllistwidget.h"
#include "pushbutton.h"
#include "modelselectable.h"
#include "model.h"

class ModelSelector : public QWidget
{
    Q_OBJECT
        
public:
    ModelSelector(QWidget* parent = 0);
    ~ModelSelector();
    void addTemplateModel();
private:
    // UI elements
    QVBoxLayout* _stretchLayout = nullptr;
    PushButton* _addNewObjectBtn = nullptr;
    PushButton* _addTemplateObjectBtn = nullptr;
    ScrollListWidget* _objectList = nullptr;

    // Object selection storage
    QList<ModelSelectable*> _objectSelectables;
    void addNewObjectByPath(QString path);
private:
    // Slot functions
    void addNewObject();

    // Util functions
    QString getNewObjectPath();
    Model* loadObject(const QString& path);
    void removeObject(ModelSelectable* selectable);

signals:
    void onObjectSelected(Model* selectedModel);
};
