//
// Created by Administrator on 2025/8/23.
//

#ifndef OPENGLRENDER_PREZPASS_H
#define OPENGLRENDER_PREZPASS_H

#include "../RenderPass.h"
#include "../../helper/Scene.h"

class PreZPass  final : public RenderPass{
public:
    explicit PreZPass(Scene &scene);

    void init(RenderResource &resource) override;

    void render(RenderResource &resource) override;
private:
    Shader shader;
    Scene &scene;
    GLuint fbo = 0;
    GLuint depthTexture = 0;
};


#endif //OPENGLRENDER_PREZPASS_H