//
// Created by Administrator on 2025/7/26.
//

#include "PostprocessPass.h"
#include "../utils/checkGlCommand.h"

PostprocessPass::PostprocessPass(Scene &scene):RenderPass("Postprocess"),scene(scene) ,postShader("shader/postprocess.vert", "shader/postprocess.frag"){

}

void PostprocessPass::init(RenderResource &resource) {
    RenderPass::init(resource);
}
void PostprocessPass::render(RenderResource &resource) {
    if (!isInit){
        std::cout << "PostprocessPass not init!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 默认 FBO（屏幕）
    postShader.bind();

    // VP
    postShader.setMat4("projection", scene.camera->getProjectionMatrix());
    postShader.setMat4("view", scene.camera->getViewMatrix());
    // 摄像机
    GL_CALL(postShader.setVec3("camPos", scene.camera->Position));
    // 灯光
    if (!scene.lights.empty()) {
        GL_CALL(postShader.setVec3("lightPos", scene.lights[0]->position));
        GL_CALL(postShader.setVec3("lightColor", scene.lights[0]->color));
    }
    //SSR
    postShader.setInt("EnableSSR", scene.EnableSSR?1:0);
    postShader.setInt("totalStepTimes",scene.totalStepTimes);
    postShader.setFloat("stepSize",scene.stepSize);
    postShader.setFloat("EPS",scene.EPS);
    postShader.setFloat("threshold",scene.threshold);
    postShader.setFloat("SSRStrength",scene.SSRStrength);
    // gBuffer
    GL_CALL(postShader.setInt("gPosition", 0));
    GL_CALL(postShader.setInt("gNormal", 1));
    GL_CALL(postShader.setInt("gAlbedo", 2));
    GL_CALL(postShader.setInt("gMaterial", 3));
    GL_CALL(postShader.setInt("gEmission", 4));
    GL_CALL(postShader.setInt("gDepth", 5));
    GL_CALL(postShader.setInt("shadowMap", 6));
    GL_CALL(postShader.setInt("lightTexture", 7));
    // 绑定 G-Buffer纹理
    GL_CALL(glActiveTexture(GL_TEXTURE0));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gPosition"]));
    GL_CALL(glActiveTexture(GL_TEXTURE1));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gNormal"]));
    GL_CALL(glActiveTexture(GL_TEXTURE2));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gAlbedo"]));
    GL_CALL(glActiveTexture(GL_TEXTURE3));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gMaterial"]));
    GL_CALL(glActiveTexture(GL_TEXTURE4));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gEmission"]));
    GL_CALL(glActiveTexture(GL_TEXTURE5));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gDepth"]));
    GL_CALL(glActiveTexture(GL_TEXTURE6));GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["shadowMap"]));
    // 光照结果
    glActiveTexture(GL_TEXTURE7);glBindTexture(GL_TEXTURE_2D, resource.textures["lightTexture"]);
    glBindVertexArray(resource.VAOs["quad"]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    postShader.unBind();
}
