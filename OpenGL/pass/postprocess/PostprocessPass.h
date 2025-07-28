//
// Created by Administrator on 2025/7/26.
//

#ifndef OPENGLRENDER_POSTPROCESSPASS_H
#define OPENGLRENDER_POSTPROCESSPASS_H


#include "../RenderPass.h"
#include "../../helper/Scene.h"

class PostprocessPass : public RenderPass{
public:
    PostprocessPass(Scene &scene,const Shader &postShader);

    void init(RenderResource& resource) override;
    void render(RenderResource& resource) override;
    int bindParams(RenderResource &resource);
    void toScreen();
    Shader postShader;
    Scene &scene;
private:
    GLuint renderedTexture = 0;
    GLuint postFBO = 0;
};


#endif //OPENGLRENDER_POSTPROCESSPASS_H
