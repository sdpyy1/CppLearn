#ifndef OPENGLRENDER_SHADOWPASS_H
#define OPENGLRENDER_SHADOWPASS_H
#include <glad/glad.h>

#include "RenderPass.h"
#include "../helper/Scene.h"

class ShadowPass : public RenderPass{
public:
    explicit ShadowPass(Scene &scene);
    GLuint shadowFBO = 0, shadowMap = 0;
    Shader shadowShader;
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    GLuint debugQuadVAO = 0 , debugQuadVBO = 0;
    Shader debugShader;
    void init(RenderResource& resource) override;
    void render(RenderResource& resource) override;
    void renderDebug();
private:
    Scene &scene;

    void initDebug();


};


#endif //OPENGLRENDER_SHADOWPASS_H
