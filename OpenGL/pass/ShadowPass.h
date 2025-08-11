#ifndef OPENGLRENDER_SHADOWPASS_H
#define OPENGLRENDER_SHADOWPASS_H
#include <glad/glad.h>

#include "RenderPass.h"
#include "../helper/Scene.h"

class ShadowPass : public RenderPass {
public:
    explicit ShadowPass(Scene &scene);

    void init(RenderResource &resource) override;

    void render(RenderResource &resource) override;

private:
    Scene &scene;
    GLuint shadowFBO = 0, shadowMap = 0;
    Shader shadowShader;
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    int shadowMapWidth = scene.width * 2;
    int shadowMapHeight = scene.height * 2;

    // 光源视角摄像机设置
    float orthoSize = 10.0f;
    float near_plane = 1.f;
    float far_plane = 50.0f;
};


#endif //OPENGLRENDER_SHADOWPASS_H
