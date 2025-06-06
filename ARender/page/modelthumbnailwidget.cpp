#include <qfile.h>
#include <qdir.h>
#include <algorithm>

#include "modelthumbnailwidget.h"

ModelThumbnailWidget::ModelThumbnailWidget(Model* model, QWidget* parent) : 
    QOpenGLWidget(parent), _model(model)
{
    // OpenGL initialize
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 3);
    setFormat(format);
}

ModelThumbnailWidget::~ModelThumbnailWidget() {
}


void ModelThumbnailWidget::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    _shaderProgram.ensureInitialized();
    
    VertexShader vertexShader("./shaders/thumbnailvertexshader.glsl");
    FragmentShader fragmentShader("./shaders/thumbnailfragmentshader.glsl");
    _shaderProgram.attachShader(vertexShader);
    _shaderProgram.attachShader(fragmentShader);
    vertexShader.dispose();
    fragmentShader.dispose();

    _object.setModel(_model->copyToCurrentContext());
    _object.updateBoundary();
    
    // Move the camera to the right position
    float viewportWidth = width();
    float viewportHeight = height();
    float fovy = _camera.fovy();
    float fovx = 2.0f * atanf(tanf(fovy / 2.0f) * viewportWidth / viewportHeight);
    float objectHeight = _object.boundary().topControlPoint().y - _object.boundary().bottomControlPoint().y;
    float objectWidth = _object.boundary().topControlPoint().x - _object.boundary().bottomControlPoint().x;
    float distance = std::max(objectHeight / (2 * tan(fovy / 2)), objectWidth / (2 * tan(fovx / 2)));
    glm::vec2 center = _object.boundary().centerPoint();
    _camera.setPosition(glm::vec3(center, _object.boundary().topControlPoint().z + distance + 3.0f));
    view = _camera.viewMatrix();
    projection = _camera.projectionMatrix((float)width() / (float)height());
    // 只有一个shader不需要切换
    _shaderProgram.bind();

}

void ModelThumbnailWidget::paintGL() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _shaderProgram.setUniform("view", view);
    _shaderProgram.setUniform("projection", projection);
    _object.render(_shaderProgram);

}

void ModelThumbnailWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}
