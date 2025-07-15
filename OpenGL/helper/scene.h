//
// Created by Administrator on 2025/7/14.
//

#ifndef OPENGLRENDER_SCENE_H
#define OPENGLRENDER_SCENE_H


#include <vector>
#include <memory>
#include "../model/model.cpp"
#include "../core/camera.h"
#include "Light.h"

class Scene {
public:
    int width;
    int height;
    std::vector<Model> models;
    std::vector<std::shared_ptr<Light>> lights;
    Camera * camera = nullptr;
    explicit Scene(Camera * camera){
        width = camera->width;
        height = camera->height;
        this->camera = camera;
    }
    void addModel(const Model& model) { models.push_back(model); }
    void addLight(const std::shared_ptr<Light>& light) {
        lights.push_back(light);
    }
    void drawAll(Shader& shader) {
        setVPAndUseShader(shader);
        for (auto& model : models) {
            shader.setMat4("model", model.modelMatrix);
            model.draw(shader);
        }
    }

    inline void setVPAndUseShader(Shader& shader) const {
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 view = camera->GetViewMatrix();

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        shader.setVec3("camPos", camera->Position);
        shader.setVec3("lightPos", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    }


};


#endif //OPENGLRENDER_SCENE_H
