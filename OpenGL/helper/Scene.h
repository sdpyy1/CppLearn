#ifndef OPENGLRENDER_SCENE_H
#define OPENGLRENDER_SCENE_H

#include <vector>
#include <memory>
#include "../model/Model.h"
#include "../core/Camera.h"
#include "Light.h"
#include "../core/Shader.h"

class Scene
{
public:
    int width;
    int height;
    std::vector<Model> models;
    std::vector<std::shared_ptr<Light>> lights;
    Camera* camera = nullptr;
    GLuint cubeVAO = 0;
    GLuint cubeVBO = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    explicit Scene(Camera* camera);

    void addModel(const Model& model);
    void addLight(const std::shared_ptr<Light>& light);

    void drawAll(Shader& shader);
    void setVPAndUseShader(Shader& shader) const;
    void renderCube();
    void renderQuad();

    GLuint loadCubemap(const char *path);
};

#endif //OPENGLRENDER_SCENE_H
