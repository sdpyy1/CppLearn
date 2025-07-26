//
// Created by Administrator on 2025/7/26.
//

#ifndef OPENGLRENDER_POSTPROCESSPASS_H
#define OPENGLRENDER_POSTPROCESSPASS_H


#include "RenderPass.h"
#include "../helper/Scene.h"

class PostprocessPass : public RenderPass{
public:
    explicit PostprocessPass(Scene &scene);

    void init(RenderResource& resource) override;
    void render(RenderResource& resource) override;
private:
    Scene &scene;
    Shader postShader;
    GLuint quadVAO = 0;

    void initQuad();
};


#endif //OPENGLRENDER_POSTPROCESSPASS_H
