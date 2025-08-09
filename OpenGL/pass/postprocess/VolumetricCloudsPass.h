#ifndef VOLUMETRICCLOUDSPASS_H
#define VOLUMETRICCLOUDSPASS_H
#include "PostprocessPass.h"

class VolumetricCloudsPass :public PostprocessPass{
public:
    explicit VolumetricCloudsPass(Scene& scene)
            : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/volumetricClouds.frag")) {
    }
    void GUIRender() override {

    }
    void render(RenderResource& resource) override {
        int nextFreeTextureId = bindParams(resource);
        // 参数设置


        PostprocessPass::render(resource);
        postShader.unBind();
    }

    void init(RenderResource& resource) override {
        passName = "VolumetricCloudsPass";
        PostprocessPass::init(resource);
    }
};
#endif //VOLUMETRICCLOUDSPASS_H
