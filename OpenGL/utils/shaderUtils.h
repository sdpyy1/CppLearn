#pragma once

#include "../core/Shader.h"
#include "../core/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
inline void handlerShader(Shader& shader, const Camera& camera, int width, int height, const glm::mat4& model = glm::mat4(1.0f)) {
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);
    shader.setVec3("camPos", camera.Position);
    shader.setVec3("lightPos", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
}