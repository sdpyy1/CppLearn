#ifndef OPENGLRENDER_FINALCOLORPASS_H
#define OPENGLRENDER_FINALCOLORPASS_H

#include "imgui.h"
#include "PostprocessPass.h"

class FinalColorPass : public PostprocessPass {
public:
    explicit FinalColorPass(Scene &scene)
        : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/finalColor.frag")) {
    }

    void init(RenderResource &resource) override {
        passName = "ToneMapping";
        PostprocessPass::init(resource);
    }

    void GUIRender() override {
        ImGui::RadioButton("None", &scene.toneMappingType, 0);
        ImGui::SameLine(); // 可选：让按钮横向排列
        ImGui::RadioButton("Uncharted 2", &scene.toneMappingType, 1);
        ImGui::SameLine();
        ImGui::RadioButton("ACES Film", &scene.toneMappingType, 2);
    }

    void render(RenderResource &resource) override {
        int nextFreeTextureId = bindParams(resource);
        postShader.setInt("toneMappingType", scene.toneMappingType);
        PostprocessPass::render(resource);
        postShader.unBind();
    }
};

#endif //OPENGLRENDER_FINALCOLORPASS_H
