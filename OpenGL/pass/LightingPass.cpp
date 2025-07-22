#include "LightingPass.h"
#include "../utils/checkGlCommand.h"

LightingPass::LightingPass(Scene &s, GLuint gPosition, GLuint gNormal, GLuint gAlbedo, GLuint gMaterial,GLuint gEmission,GLuint gDepth)
        : lightingShader("shader/pbr_gBuffer.vert", "shader/pbr_gBuffer.frag"),
          gPosition(gPosition), gNormal(gNormal), gAlbedo(gAlbedo), gMaterial(gMaterial), gEmission(gEmission),gDepth(gDepth),scene(s), quadVAO(0) {}

void LightingPass::init() {
    initQuad();  // 创建全屏 quad VAO
    isInit = true;
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
    if (!isInit){
        std::cerr << "LightingPass not init!" << std::endl;
        return;
    }
    GL_CALL(lightingShader.bind());

    // 设置 uniforms
    GL_CALL(lightingShader.setInt("gPosition", 0));
    GL_CALL(lightingShader.setInt("gNormal", 1));
    GL_CALL(lightingShader.setInt("gAlbedo", 2));
    GL_CALL(lightingShader.setInt("gMaterial", 3));
    GL_CALL(lightingShader.setInt("gEmission", 4));
    GL_CALL(lightingShader.setInt("gDepth", 5));
    GL_CALL(lightingShader.setVec3("camPos", scene.camera->Position));

    if (!scene.lights.empty()) {
        GL_CALL(lightingShader.setVec3("lightPos", scene.lights[0]->position));
        GL_CALL(lightingShader.setVec3("lightColor", scene.lights[0]->color));
    }


    // 绑定 G-Buffer
    GL_CALL(glActiveTexture(GL_TEXTURE0));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gPosition));

    GL_CALL(glActiveTexture(GL_TEXTURE1));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gNormal));

    GL_CALL(glActiveTexture(GL_TEXTURE2));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gAlbedo));

    GL_CALL(glActiveTexture(GL_TEXTURE3));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gMaterial));

    GL_CALL(glActiveTexture(GL_TEXTURE4));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gEmission));
    GL_CALL(glActiveTexture(GL_TEXTURE5));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gDepth));

    GL_CALL(glBindVertexArray(quadVAO));
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_CALL(glBindVertexArray(0));
}

