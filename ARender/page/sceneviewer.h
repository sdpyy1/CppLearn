#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H
#include <QOpenGLFunctions>
#include <QtOpenGLWidgets/qopenglwidget.h>

class SceneViewer:public QOpenGLWidget,QOpenGLFunctions
{
    Q_OBJECT;
public:
    SceneViewer(QWidget* parent = 0);

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
};

#endif // SCENEVIEWER_H
