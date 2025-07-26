//
// Created by Administrator on 2025/7/26.
//

#include "PostprocessPass.h"
#include "../utils/checkGlCommand.h"

PostprocessPass::PostprocessPass(Scene &scene):RenderPass("Postprocess"),scene(scene) ,postShader("shader/postprocess.vert", "shader/postprocess.frag"){

}

void PostprocessPass::init(RenderResource &resource) {
    initQuad();
    RenderPass::init(resource);
}
void PostprocessPass::initQuad() {
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
void PostprocessPass::render(RenderResource &resource) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 默认 FBO（屏幕）
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    postShader.bind();
    // 绑定 lightingResultTex
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, resource.textures["lightTexture"]);
    GL_CALL(postShader.setInt("gDepth", 1));
    GL_CALL(glActiveTexture(GL_TEXTURE1));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gDepth"]));
    postShader.setInt("lightTexture", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    postShader.unBind();
}
