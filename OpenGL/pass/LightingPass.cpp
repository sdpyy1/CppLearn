#include "LightingPass.h"

LightingPass::LightingPass(Scene &s, GLuint gPosition, GLuint gNormal, GLuint gAlbedo, GLuint gMaterial)
        : lightingShader("shader/phong.vert", "shader/phong.frag"),
          gPosition(gPosition), gNormal(gNormal), gAlbedo(gAlbedo), gMaterial(gMaterial),
          scene(s), quadVAO(0) {}

void LightingPass::init() {
    initQuad();  // 创建全屏 quad VAO
}

void LightingPass::initQuad() {
    float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };

    GLuint quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // texCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void LightingPass::render() {
    lightingShader.use();

    // 设置 uniform
    lightingShader.setInt("gPosition", 0);
    lightingShader.setInt("gNormal", 1);
    lightingShader.setInt("gAlbedo", 2);
    lightingShader.setInt("gMaterial", 3);
    lightingShader.setVec3("viewPos", scene.camera->Position);
    lightingShader.setInt("lightCount", scene.lights.size());

    for (int i = 0; i < scene.lights.size(); ++i) {
        std::string base = "lights[" + std::to_string(i) + "]";
        lightingShader.setVec3(base + ".position", scene.lights[i]->position);
        lightingShader.setVec3(base + ".color", scene.lights[i]->color);
    }

    // 绑定 G-Buffer
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, gMaterial);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
