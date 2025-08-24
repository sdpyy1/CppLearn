#include "GeometryPass.h"

GeometryPass::GeometryPass(Scene &scene)
    : RenderPass("GeometryPass"),
      shader("shader/geometry.vert", "shader/geometry.frag"),
      scene(scene) {
}

void GeometryPass::init(RenderResource &resource) {
    GL_CALL(glGenFramebuffers(1, &gBuffer));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));
    GL_CALL(glViewport(0, 0, scene.width, scene.height));
    // 1. Position (RGB16F)
    GL_CALL(glGenTextures(1, &gPosition));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scene.width, scene.height, 0, GL_RGB, GL_FLOAT, nullptr));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0));
    // 2. Normal (RGB16F)
    GL_CALL(glGenTextures(1, &gNormal));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scene.width, scene.height, 0, GL_RGB, GL_FLOAT, nullptr));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0));
    // 3. Albedo (RGBA8)
    GL_CALL(glGenTextures(1, &gAlbedo));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scene.width, scene.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0));
    // 4. Material info (RGBA8)
    GL_CALL(glGenTextures(1, &gMaterial));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, gMaterial));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scene.width, scene.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMaterial, 0));
    // 5. Emission info (RGBA8)
    glGenTextures(1, &gEmission);
    glBindTexture(GL_TEXTURE_2D, gEmission);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scene.width, scene.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gEmission, 0));
    // Depth renderbuffer
    // pre-z关闭
    // glGenTextures(1, &gDepth);
    // glBindTexture(GL_TEXTURE_2D, gDepth);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, scene.width, scene.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
    //              nullptr);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

    // pre-z开启
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, resource.textures["gDepth"], 0);


    // attach 到 FBO 的 color attachmentN，比如 GL_COLOR_ATTACHMENT4
    // TODO:添加了gBuffer缓冲后，必须添加在这
    const GLuint attachments[5] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4
    };
    glDrawBuffers(5, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }
    resource.textures["gPosition"] = gPosition;
    resource.textures["gNormal"] = gNormal;
    resource.textures["gAlbedo"] = gAlbedo;
    resource.textures["gMaterial"] = gMaterial;
    resource.textures["gEmission"] = gEmission;
    resource.framebuffers["gBuffer"] = gBuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderPass::init(resource);
}


void GeometryPass::render(RenderResource &resource) {
    shader.bind();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glViewport(0, 0, scene.width, scene.height);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    GL_CALL(scene.drawAll(shader));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    shader.unBind();
}
