#include "../utils/DefaultSceneSetup.h"
#include <glm/glm.hpp>


DefaultSceneSetup::DefaultSceneSetup(Camera &cam, int width, int height)
        : camera(cam), width(width), height(height)
{
    setupFloorMesh();
}

void DefaultSceneSetup::setupFloorMesh() {
    float vertices[] = {
            // positions           // normals         // texcoords
            5.0f, -0.5f,  5.0f,   0.0f, 1.0f, 0.0f,   5.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
            5.0f, -0.5f, -5.0f,   0.0f, 1.0f, 0.0f,   5.0f, 5.0f
    };
    unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    vao.Bind();
    vbo.SetData(vertices, sizeof(vertices));
    ibo.SetData(indices, sizeof(indices));

    vao.AddVertexAttrib(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);                   // pos
    vao.AddVertexAttrib(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    vao.AddVertexAttrib(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // uv
}

void DefaultSceneSetup::draw(Shader& shader) {
    // 设置 PBR 材质参数（除 Albedo 使用贴图外，其它为常量）
    shader.setFloat("metallic", 1.f);   // 非金属
    shader.setFloat("roughness", 0.5f);  // 半光滑
    shader.setFloat("ao", 0.0f);         // 环境光遮蔽强度
    shader.setVec3("camPos", camera.Position);
    shader.setVec3("texture_albedo",glm::vec3 (1.0, 0.6, 0.8));
    shader.setVec3("lightPos",glm::vec3 (1.0, 1, 1));
    shader.setVec3("lightColor",glm::vec3 (1,1,1));

    // 设置 MVP 矩阵
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);
    vao.Bind();
    ibo.Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    vao.Unbind();

    glBindTexture(GL_TEXTURE_2D, 0); // 可选：解绑纹理，保持状态整洁
}

