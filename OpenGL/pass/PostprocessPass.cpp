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
    // 绑定光照结果贴图
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, resource.textures["lightTexture"]);
    GL_CALL(postShader.setInt("gDepth", 1));
    GL_CALL(glActiveTexture(GL_TEXTURE1));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resource.textures["gDepth"]));
    postShader.setInt("lightTexture", 0);
    glBindVertexArray(resource.VAOs["quad"]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    postShader.unBind();
}
