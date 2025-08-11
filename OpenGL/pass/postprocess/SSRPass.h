#ifndef OPENGLRENDER_SSRPASS_H
#define OPENGLRENDER_SSRPASS_H

#include "PostprocessPass.h"
#include "imgui.h"

class SSRPass : public PostprocessPass {
public:
    explicit SSRPass(Scene &scene)
        : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/ssrShader.frag")) {
    }

    void GUIRender() override {
        ImGui::SliderInt("Total Step Times", &scene.totalStepTimes, 1, 100);
        ImGui::SliderFloat("Step Size", &scene.stepSize, 0.01f, 1.f, "%.3f");
        ImGui::SliderFloat("Threshold", &scene.threshold, 0.01f, 2.f, "%.3f");
        ImGui::SliderFloat("strength", &scene.SSRStrength, 0.1f, 2.f, "%.3f");
    }

    void render(RenderResource &resource) override {
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

    void init(RenderResource &resource) override {
        passName = "SSR";
        PostprocessPass::init(resource);
    }
};

#endif //OPENGLRENDER_SSRPASS_H
