#include "sceneviewer.h"

SceneViewer::SceneViewer(QWidget *parent):QOpenGLWidget(parent){
    // OpenGL initialize
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    setFormat(format);
    this->resize(400,400);
}

void SceneViewer::initializeGL(){
    this->initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneViewer::resizeGL(int w, int h){
    glViewport(0, 0, w, h);
}

void SceneViewer::paintGL(){
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
