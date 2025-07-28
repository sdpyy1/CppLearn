#ifndef OPENGLRENDER_FINALCOLORPASS_H
#define OPENGLRENDER_FINALCOLORPASS_H

#include "PostprocessPass.h"

class FinalColorPass : public PostprocessPass {
public:
    explicit FinalColorPass(Scene& scene)
            : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/finalColor.frag")) {
    }

    void init(RenderResource& resource) override {
        passName = "finalColor_Pass";
        PostprocessPass::init(resource);
    }

    void render(RenderResource& resource) override {
        int nextFreeTextureId = bindParams(resource);

        postShader.setInt("toneMappingType", scene.toneMappingType);
        // 强制输出到屏幕
        toScreen();
        PostprocessPass::render(resource);
        postShader.unBind();
    }
};

#endif //OPENGLRENDER_FINALCOLORPASS_H