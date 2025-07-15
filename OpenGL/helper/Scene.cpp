#include "Scene.h"

Scene::Scene(Camera* camera)
{
    width = camera->width;
    height = camera->height;
    this->camera = camera;
}

void Scene::addModel(const Model& model)
{
    models.push_back(model);
}

void Scene::addLight(const std::shared_ptr<Light>& light)
{
    lights.push_back(light);
}

void Scene::drawAll(Shader& shader)
{
    setVPAndUseShader(shader);
    for (auto& model : models)
    {
        shader.setMat4("model", model.modelMatrix);
        model.draw(shader);
    }
}

void Scene::setVPAndUseShader(Shader& shader) const
{
    shader.use();
    glm::mat4 projection = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();

    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    shader.setVec3("camPos", camera->Position);
    // shader.setVec3("lightPos", glm::vec3(1.0f, 1.0f, 1.0f));
    // shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
}
