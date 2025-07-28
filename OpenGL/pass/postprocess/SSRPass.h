#ifndef OPENGLRENDER_SSRPASS_H
#define OPENGLRENDER_SSRPASS_H

#include "PostprocessPass.h"

class SSRPass : public PostprocessPass {
public:
    explicit SSRPass(Scene& scene)
            : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/ssrShader.frag")) {
    }

    void render(RenderResource& resource) override {
        int nextFreeTextureId = bindParams(resource);
        // SSR参数设置
        postShader.setInt("EnableSSR", scene.EnableSSR ? 1 : 0);
        postShader.setInt("totalStepTimes", scene.totalStepTimes);
        postShader.setFloat("stepSize", scene.stepSize);
        postShader.setFloat("EPS", scene.EPS);
        postShader.setFloat("threshold", scene.threshold);
        postShader.setFloat("SSRStrength", scene.SSRStrength);

        PostprocessPass::render(resource);
        postShader.unBind();
    }

    void init(RenderResource& resource) override {
        passName = "SSR_Pass";
        PostprocessPass::init(resource);
    }
};

#endif //OPENGLRENDER_SSRPASS_H