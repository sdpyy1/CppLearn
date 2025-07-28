#ifndef OPENGLRENDER_BLOOMPASS_H
#define OPENGLRENDER_BLOOMPASS_H

#include "PostprocessPass.h"

class BloomPass : public PostprocessPass {
public:
    explicit BloomPass(Scene& scene)
            : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/bloom.frag")) {
    }

    void init(RenderResource& resource) override {
        passName = "Bloom_pass";
        PostprocessPass::init(resource);
    }

    void render(RenderResource& resource) override {
        int nextFreeTextureId = bindParams(resource);
        PostprocessPass::render(resource);
        postShader.unBind();
    }
};

#endif //OPENGLRENDER_BLOOMPASS_H