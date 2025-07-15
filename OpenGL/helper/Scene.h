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

    explicit Scene(Camera* camera);

    void addModel(const Model& model);
    void addLight(const std::shared_ptr<Light>& light);

    void drawAll(Shader& shader);
    void setVPAndUseShader(Shader& shader) const;
};

#endif //OPENGLRENDER_SCENE_H
