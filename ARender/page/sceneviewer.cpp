#include "sceneviewer.h"

#include <string>
#include <qresource.h>
#include <qurl.h>
#include <qdir.h>

#include "illuminer.h"

SceneViewer::SceneViewer(QWidget* parent)
    : QOpenGLWidget(parent)
{
    // Set mouse tracking
    setMouseTracking(true);
    // Set key tracking
    setFocusPolicy(Qt::StrongFocus);
    // Set the focus
    setFocus();

    // OpenGL initialize
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 3);
    setFormat(format);

}

void SceneViewer::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    Logger::info("OpenGL版本: " + std::string((const char*)glGetString(GL_VERSION)));
    // 默认天空盒
    _sky = new SkyBox("./assets/skybox");
    // 加载地板
    _plane = new Plane("./assets/diffuse.png");
    // 加载shader
    _shaderProgram = ShaderProgram("./shaders/model.vert","./shaders/model.frag");
    _boundShader = ShaderProgram("./shaders/boundvertexshader.glsl","./shaders/boundfragmentshader.glsl");
    _skyShader = ShaderProgram("./shaders/skyboxvertexshader.glsl","./shaders/skyboxfragmentshader.glsl");
    _terrainShader = ShaderProgram("./shaders/terrainvertexshader.glsl","./shaders/terrainfragmentshader.glsl");

    // 摄像机
    _camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));

    DirLight * dirLightTest = new DirLight({3,3,3},{1,1,1});
    _lightList.push_back(dirLightTest);
    _dirLightCount ++;
}

// 调整窗口
void SceneViewer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

// 渲染
void SceneViewer::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set view and projection matrices
    glm::mat4 view = _camera.viewMatrix();
    glm::mat4 projection = _camera.projectionMatrix((float)width() / (float)height());

    // Terrain Render ---------------------------------------------------
    if (_terrain != nullptr) {
        _terrainShader.bind();
        _terrainShader.setUniform("view", view);
        _terrainShader.setUniform("projection", projection);
        _terrainShader.setUniform("model", _terrain->modelMatrix());
        _terrainShader.setUniform("texture1", 2);
        _terrain->render();
        _terrainShader.unbind();
    }
    // ------------------------------------------------------------------

    // Renderable Render ------------------------------------------------
    _shaderProgram.bind();
    _shaderProgram.setUniform("view", view);
    _shaderProgram.setUniform("projection", projection);
    int curDirLightCount = 0;
    int curScopeLightCount = 0;
    int curPointLightCount = 0;
    // 设置光照
    if(_haveNewLight){
        _haveNewLight = false;
        for(auto &light : _lightList){
            Logger::debug("{}",light->getType());
            if(light->getType() == 0){
                // 点光源
                light->updateShader(_shaderProgram,curPointLightCount++);
            }else if(light->getType() == 1){
                // 平行光
                light->updateShader(_shaderProgram,curDirLightCount++);
            }else if(light->getType() == 2){
                // 聚光
                light->updateShader(_shaderProgram,curScopeLightCount++);
            }
        }
    }

    _shaderProgram.setUniform("dirlightnr",_dirLightCount);
    _shaderProgram.setUniform("pointlightnr",_pointLightCount);
    _shaderProgram.setUniform("spotlightnr",_scopedLightCount);
    // Render objects
    for (auto object : _objects) {
        object->render(_shaderProgram);
    }
    // 渲染地板
    if(_needPlane){
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,{0,-10,-30});
        _shaderProgram.setUniform("model", model);
        _plane->render();
    }
    _shaderProgram.unbind();
    // ------------------------------------------------------------------

    // Bound box render -------------------------------------------------
    if (_selectedObject != nullptr && !_hideBound) {
        _boundShader.bind();
        _boundShader.setUniform("view", view);
        _boundShader.setUniform("projection", projection);
        _selectedObject->boundary().render();
        _boundShader.unbind();
    }
    if (_hoveredObject != nullptr && _hoveredObject != _selectedObject) {
        _boundShader.bind();
        _boundShader.setUniform("view", view);
        _boundShader.setUniform("projection", projection);
        _hoveredObject->boundary().render();
        _boundShader.unbind();
    }
    // ------------------------------------------------------------------


    // Sky Box Render ---------------------------------------------------
    if (_sky != nullptr) {
        _skyShader.bind();
        _skyShader.setUniform("view", glm::mat4(glm::mat3(view)));
        _skyShader.setUniform("projection", projection);
        _sky->render();
        _skyShader.unbind();
    }
    // ------------------------------------------------------------------

}

void SceneViewer::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        _pressedObject = _hoveredObject;
    }
    else {
        _lastMousePosition = event->pos();
    }

    parentWidget()->update();
    setFocus();
}
SceneViewer::~SceneViewer() {
    for (auto obj : _objects) {
        delete obj;
    }
}


Renderable* SceneViewer::hitTest(const Ray& ray) {
    HitRecord newRecord = HitRecord();
    Renderable* newObject = nullptr;
    // Object hit test
    for (int i = 0; i < _objects.size(); i++) {
        Logger::debug("Testing object " + std::to_string(i));
        Renderable* obj = _objects[i];
        if (obj == _operatingObject) {
            // Ignore current operating Object
            continue;
        }
        HitRecord hitRecord = obj->hit(ray);
        if (hitRecord.hitted()) {
            Logger::debug("Hitted object " + std::to_string(i));
        }
        else {
            Logger::debug("Missed object " + std::to_string(i));
        }
        if (hitRecord.hitted() && hitRecord.t() < newRecord.t()) {
            newRecord = hitRecord;
            newObject = obj;
        }
    }
    // Terrain hit test
    if (_terrain != nullptr) {
        HitRecord hitRecord = _terrain->hit(ray);
        if (hitRecord.hitted()) {
            Logger::debug("Hitted terrain");
        }
        else {
            Logger::debug("Missed terrain");
        }
        if (hitRecord.hitted() && hitRecord.t() < newRecord.t()) {
            newRecord = hitRecord;
            newObject = nullptr;
        }
    }
    _hitRecord = newRecord;
    return newObject;
}
void SceneViewer::mouseReleaseEvent(QMouseEvent* event) {
    // State transfer
    bool startOperatingObject = false;
    if (_operatingObject != nullptr) {
        // Click when having an operating object
        _operatingObject->updateBoundary();
        if (!_dragged) {
            // if haven't changed since last mouse press, it's a submission click
            _operatingObject = nullptr;
            _hideBound = false;
        }
        else {
            // dragged, keep it operational
            _dragged = false;
            _hideBound = true;
            _operatingObject = _operatingObject;
        }
    }
    else if (_pressedObject != nullptr && _pressedObject == _selectedObject) {
        if (!_dragged) {
            // Double select on an object, set in operating mode
            _operatingObject = _selectedObject;
            _hideBound = true;
            startOperatingObject = true;
        }
        else {
            // keep it selected
            _dragged = false;
            _hideBound = false;
            _selectedObject->updateBoundary();
        }
    }
    else if (_dragged) {
        _dragged = false;
        _hideBound = false;
        if (_selectedObject != nullptr) {
            _selectedObject->updateBoundary();
        }
    }
    else {
        _selectedObject = _pressedObject;
        _hideBound = false;
        emit onSelect(_selectedObject);
    }

    // Reset pressed object
    _pressedObject = nullptr;

    // Update hover object
    float relX = (float)event->x() / (float)width();
    float relY = 1 - (float)event->y() / (float)height();
    Ray ray = _camera.generateRay(glm::vec2(relX, relY), (float)width() / (float)height());
    _hoveredObject = hitTest(ray);

    if (startOperatingObject) {
        // If just setted to operating mode, move the object
        moveOperatingObject(ray);
    }

    // Update the view
    parentWidget()->update();
}

void SceneViewer::mouseMoveEvent(QMouseEvent* event) {
    // Check the type of button pressed
    switch (event->buttons()) {
        case Qt::LeftButton: {
            if (_selectedObject != nullptr) {
                // Set dragged
                _dragged = true;
                // Hide boundary
                _hideBound = true;
                // Rotate around camera up
                glm::vec2 delta = glm::vec2(event->x() - _lastMousePosition.x(), event->y() - _lastMousePosition.y());
                _selectedObject->rotate(_camera.up(), delta.x * 1.0f);
                // Rotate around camera right
                _selectedObject->rotate(_camera.right(), delta.y * 1.0f);
                emit onUpdate(_selectedObject);
            }
            break;
        }
        case Qt::RightButton: {
            // Set dragged
            _dragged = true;
            moveCamera(event);
            break;
        }
        case Qt::MiddleButton: {
            if (_controlPressed && _selectedObject != nullptr) {
                // Set dragged
                _dragged = true;
                // Hide boundary
                _hideBound = true;
                // Scale object
                glm::vec2 delta = glm::vec2(event->x() - _lastMousePosition.x(), event->y() - _lastMousePosition.y());
                _selectedObject->scale(-delta.y * 0.01f);
                emit onUpdate(_selectedObject);
            }
            else {
                // Set dragged
                _dragged = true;
                rotateCamera(event);
            }
            break;
        }
        case Qt::NoButton: {
            float relX = (float)event->x() / (float)width();
            float relY = 1 - (float)event->y() / (float)height();
            Ray ray = _camera.generateRay(glm::vec2(relX, relY), (float)width() / (float)height());
            if (_operatingObject == nullptr) {
                // If no button pressed, do hit test and move the current object if selected
                _hoveredObject = hitTest(ray);
                if (_hoveredObject != nullptr) {
                    setCursor(Qt::PointingHandCursor);
                }
                else {
                    setCursor(Qt::ArrowCursor);
                }
            }
            else {
                moveOperatingObject(ray);
                emit onUpdate(_selectedObject);
            }
            break;
        }
        default: {
            Logger::warning("Unknown mouse button input");
            Logger::warning("Mouse button: " + std::to_string(event->buttons()));
            break;
        }
    }
    // Update the last mouse position
    _lastMousePosition = event->pos();
    // Update the view
    parentWidget()->update();
}

void SceneViewer::wheelEvent(QWheelEvent* event) {
    // Zoom in or out
    float wheelOffset = event->angleDelta().y();
    Logger::debug("Wheel offset: " + std::to_string(wheelOffset));
    _camera.push(wheelOffset * _cameraPushSpeed);
    glm::vec3 cameraFront = _camera.front();
    _rotateCenter += wheelOffset * _cameraPushSpeed * cameraFront;
    Logger::debug("New camera position: " + std::to_string(_camera.position().x) + ", " + std::to_string(_camera.position().y) + ", " + std::to_string(_camera.position().z));
    Logger::debug("New center position: " + std::to_string(_rotateCenter.x) + ", " + std::to_string(_rotateCenter.y) + ", " + std::to_string(_rotateCenter.z));
    // Update the view
    parentWidget()->update();
}

void SceneViewer::moveCamera(QMouseEvent* event) {
    // Move the camera
    float xoffset = event->x() - _lastMousePosition.x();
    float yoffset = _lastMousePosition.y() - event->y();    // reversed since y-coordinates go from bottom to top
    float xmovement = xoffset * _cameraMovementSpeed;
    float ymovement = yoffset * _cameraMovementSpeed;
    glm::vec3 cameraPrevPos = _camera.position();
    _camera.move({ -xmovement, -ymovement });
    glm::vec3 cameraNewPos = _camera.position();
    _rotateCenter += cameraNewPos - cameraPrevPos;
    Logger::debug("Camera moved to: " + std::to_string(_camera.position().x) + ", " + std::to_string(_camera.position().y) + ", " + std::to_string(_camera.position().z));
    Logger::debug("New center: " + std::to_string(_rotateCenter.x) + ", " + std::to_string(_rotateCenter.y) + ", " + std::to_string(_rotateCenter.z));
    if (_operatingObject != nullptr) {
        float relX = (float)event->x() / (float)width();
        float relY = 1 - (float)event->y() / (float)height();
        Ray ray = _camera.generateRay(glm::vec2(relX, relY), (float)width() / (float)height());
        moveOperatingObject(ray);
    }
}

void SceneViewer::rotateCamera(QMouseEvent* event) {
    // Rotate the camera
    float xoffset = event->x() - _lastMousePosition.x();
    float yoffset = _lastMousePosition.y() - event->y();    // reversed since y-coordinates go from bottom to top
    // Calculate pitch angle
    float pitch = yoffset * _cameraRotationSpeed;
    // Calculate yaw angle
    float yaw = xoffset * _cameraRotationSpeed;
    _camera.rotate(_rotateCenter, pitch, -yaw);
    Logger::debug("Camera rotated to: " + std::to_string(_camera.position().x) + ", " + std::to_string(_camera.position().y) + ", " + std::to_string(_camera.position().z));
    Logger::debug("Center at: " + std::to_string(_rotateCenter.x) + ", " + std::to_string(_rotateCenter.y) + ", " + std::to_string(_rotateCenter.z));
    if (_operatingObject != nullptr) {
        float relX = (float)event->x() / (float)width();
        float relY = 1 - (float)event->y() / (float)height();
        Ray ray = _camera.generateRay(glm::vec2(relX, relY), (float)width() / (float)height());
        moveOperatingObject(ray);
    }
}

void SceneViewer::keyPressEvent(QKeyEvent* event) {
    Logger::debug("Detect keypress " + std::to_string(event->key()));
    // If ctrl pressed
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        Logger::debug("Control pressed");
        _controlPressed = true;
    }

    if (event->modifiers() == Qt::NoModifier && _selectedObject != nullptr) {
        switch (event->key()) {
            case Qt::Key_W: {
                // move a bit upward
                glm::vec3 up = _camera.up();
                _selectedObject->move(up * 0.1f);
                _selectedObject->updateBoundary();
                break;
            }
            case Qt::Key_S: {
                // move a bit downward
                glm::vec3 up = _camera.up();
                _selectedObject->move(-up * 0.1f);
                _selectedObject->updateBoundary();
                break;
            }
            case Qt::Key_A: {
                // move a bit left
                glm::vec3 right = _camera.right();
                _selectedObject->move(-right * 0.1f);
                _selectedObject->updateBoundary();
                break;
            }
            case Qt::Key_D: {
                // move a bit right
                glm::vec3 right = _camera.right();
                _selectedObject->move(right * 0.1f);
                _selectedObject->updateBoundary();
                break;
            }
        }
        parentWidget()->update();
    }
    else if (event->modifiers() == Qt::NoModifier) {
        // move camera
        switch (event->key()) {
            case Qt::Key_W: {
                // move a bit forward
                glm::vec3 front = _camera.front();
                _camera.push(1.0f);
                _rotateCenter += front;
                break;
            }
            case Qt::Key_S: {
                // move a bit backward
                glm::vec3 front = _camera.front();
                _camera.push(-1.0f);
                _rotateCenter -= front;
                break;
            }
            case Qt::Key_A: {
                // move a bit left
                glm::vec3 right = _camera.right();
                _camera.move(-right * 0.5f);
                _rotateCenter -= right * 0.5f;
                break;
            }
            case Qt::Key_D: {
                // move a bit right
                glm::vec3 right = _camera.right();
                _camera.move(right * 0.5f);
                _rotateCenter += right * 0.5f;
                break;
            }
        }
        parentWidget()->update();
    }
}

void SceneViewer::keyReleaseEvent(QKeyEvent* event) {
    // If no control pressed
    if (!(event->modifiers().testFlag(Qt::ControlModifier))) {
        Logger::debug("Control released");
        _controlPressed = false;
    }
}

void SceneViewer::moveOperatingObject(const Ray& ray) {
    // Current moving object
    hitTest(ray);
    if (!_hitRecord.hitted()) {
        // Move to the direction of current ray
        glm::vec3 target = _camera.position() + ray.direction() * 15.0f;
        _operatingObject->setPosition(target);
        _operatingObject->updateBoundary();
    }
    // Move the object so that the bottom center of the object is at the hit point
    else if (_stickToSurface) {
        // Set the bottom center of the model at local origin
        glm::vec3 bottomCenter = _operatingObject->modelMatrix() * glm::vec4(_operatingObject->model()->boundBox().bottomCenterPoint(), 1.0f);
        glm::vec3 modelCenter = _operatingObject->modelMatrix() * glm::vec4(glm::vec3(0.0f), 1.0f);   // model center in world space

        // Rotate the model to align with the surface normal
        glm::vec3 normal = _hitRecord.normal();
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 axis = glm::normalize(glm::cross(up, normal));
        float deg = glm::degrees(glm::acos(glm::dot(up, normal)));
        _operatingObject->setRotation(axis, deg);

        // Move the model to the hit point
        glm::vec3 target = _hitRecord.position();

        glm::vec3 newCenter = target + modelCenter - bottomCenter;
        _operatingObject->setPosition(newCenter);

        // Update the boundary
        _operatingObject->updateBoundary();
    }
    else {
        // Move the object to the hit point
        Logger::debug("Hit point: " + std::to_string(_hitRecord.position().x) + ", " + std::to_string(_hitRecord.position().y) + ", " + std::to_string(_hitRecord.position().z));
        Logger::debug("Bottom center: " + std::to_string(_operatingObject->boundary().bottomCenterPoint().x) + ", " + std::to_string(_operatingObject->boundary().bottomCenterPoint().y) + ", " + std::to_string(_operatingObject->boundary().bottomCenterPoint().z));
        glm::vec3 target = _hitRecord.position();
        glm::vec3 modelCenter = _operatingObject->modelMatrix() * glm::vec4(glm::vec3(0.0f), 1.0f);   // model center in world space
        glm::vec3 bottomCenter = _operatingObject->modelMatrix() * glm::vec4(_operatingObject->model()->boundBox().bottomCenterPoint(), 1.0f);   // model center in world space
        glm::vec3 newCenter = target + modelCenter - bottomCenter;
        _operatingObject->setPosition(newCenter);

        // Update the boundary
        _operatingObject->updateBoundary();
    }
}

void SceneViewer::addObject(Model* model) {
    makeCurrent();
    Model* newModel = model->copyToCurrentContext();
    Renderable* newObject = new Renderable(newModel);
    _selectedObject = newObject;
    _operatingObject = newObject;
    _objects.push_back(newObject);
    parentWidget()->update();
    emit onSelect(_selectedObject);
}

void SceneViewer::changePlaneShow()
{
    _needPlane = !_needPlane;
    parentWidget()->update();
}

void SceneViewer::addNewLight(int type,float x, float y, float z, float r, float g, float b,float cutoff)
{
    // 场景中添加光照
    if(type == 0){
        // 点光源
        if(_pointLightCount == 5){
            Logger::info("点光源已经有{}个了",_pointLightCount);
            return;
        }
        ScopedLight * pointLight = new ScopedLight({x,y,z},{r,g,b});
        pointLight->setCutOffAngle(360);   //  大于180就识别为点光源
        _lightList.push_back(pointLight);
        _pointLightCount ++;
        _haveNewLight = true;
    }else if(type == 1){
        // 平行光
        if(_dirLightCount == 5){
            Logger::info("平行光已经有{}个了",_dirLightCount);
            return;
        }
        DirLight * dirLight = new DirLight({x,y,z},{r,g,b});
        _lightList.push_back(dirLight);
        _dirLightCount++;
        _haveNewLight = true;
    }else if(type == 2){
        // 聚光
        if(_scopedLightCount == 5){
            Logger::info("平行光已经有{}个了",_scopedLightCount);
            return;
        }
        ScopedLight * scopedLight = new ScopedLight({x,y,z},{r,g,b});
        scopedLight->setCutOffAngle(cutoff);
        _lightList.push_back(scopedLight);
        _scopedLightCount++;
        _haveNewLight = true;
    }else{
        Logger::error("光源添加失败");
    }
    parentWidget()->update();
}

void SceneViewer::deleteObject() {
    if (_selectedObject == nullptr) {
        return;
    }
    makeCurrent();
    for (auto it = _objects.begin(); it != _objects.end(); ++it) {
        if (*it == _selectedObject) {
            _objects.erase(it);
            break;
        }
    }
    delete _selectedObject;
    if (_hoveredObject == _selectedObject) {
        _hoveredObject = nullptr;
    }
    if (_pressedObject == _selectedObject) {
        _pressedObject = nullptr;
    }
    _selectedObject = nullptr;
    _operatingObject = nullptr;
    emit onSelect(nullptr);
    parentWidget()->update();
}

void SceneViewer::updateSetting(QPair<QString, QString> setting) {
    Logger::info("场景配置更新" + setting.first.toStdString() + ":" + setting.second.toStdString());
    makeCurrent();
    if (setting.first == "stickSurface") {
        if (setting.second == "true") {
            _stickToSurface = true;
        }
        else {
            _stickToSurface = false;
        }
    }
    else if (setting.first == "skybox") {
        if (_sky != nullptr) {
            delete _sky;
            _sky = nullptr;
        }
        if (!setting.second.isEmpty()) {
            _sky = new SkyBox(setting.second.toStdString());
        }
    }
    else if (setting.first == "terrain") {
        if (_terrain != nullptr) {
            delete _terrain;
            _terrain = nullptr;
        }
        if (!setting.second.isEmpty()) {
            _terrain = new Terrain(setting.second.toStdString());
        }
    }
    else {
        Logger::warning("Unknown setting input");
    }
    doneCurrent();
}

void SceneViewer::changeRenderFlag()
{
    if(_selectedObject != nullptr){
        _selectedObject->changeRenderLineFlag();
    }
    parentWidget()->update();
}
void printMat4(const glm::mat4& mat) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << mat[j][i] << " ";
        }
        std::cout << std::endl;
    }
}
