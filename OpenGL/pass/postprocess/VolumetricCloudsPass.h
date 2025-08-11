#ifndef VOLUMETRICCLOUDSPASS_H
#define VOLUMETRICCLOUDSPASS_H
#include "PostprocessPass.h"

class VolumetricCloudsPass : public PostprocessPass {
public:
    explicit VolumetricCloudsPass(Scene &scene)
        : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/volumetricClouds.frag")),
          noiseGenShader(Shader("shader/compute/volumetricClouds.comp")) {
    }

    void GUIRender() override {
    }

    void render(RenderResource &resource) override {
        int nextFreeTextureId = bindParams(resource);
        // 参数设置
        postShader.setInt("noiseTexture", nextFreeTextureId);
        glActiveTexture(GL_TEXTURE0 + nextFreeTextureId);
        glBindTexture(GL_TEXTURE_3D, noiseTexture);
        PostprocessPass::render(resource);
        postShader.unBind();
    }

    void init(RenderResource &resource) override {
        passName = "VolumetricCloudsPass";
        PostprocessPass::init(resource);
        noiseGenShader.bind();
        create3DTextureRGBA(30, 30, 30);
        glBindImageTexture(0, noiseTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        noiseGenShader.setVec3i("texSize", glm::ivec3(30, 30, 30));

        int groupX = (30 + 7) / 8;
        int groupY = (30 + 7) / 8;
        int groupZ = (30 + 7) / 8;
        glDispatchCompute(groupX, groupY, groupZ);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        noiseGenShader.unBind();
    }

    void create3DTextureRGBA(const int width, const int height, const int depth) {
        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_3D, noiseTexture);

        // 分配4通道32位浮点存储
        glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, width, height, depth);

        // 线性过滤和重复包裹
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glBindTexture(GL_TEXTURE_3D, 0);
    }

private:
    GLuint noiseTexture = 0;
    Shader noiseGenShader;
};
#endif //VOLUMETRICCLOUDSPASS_H
