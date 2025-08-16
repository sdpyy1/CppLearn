#ifndef VOLUMETRICCLOUDSPASS_H
#define VOLUMETRICCLOUDSPASS_H
#include "PostprocessPass.h"

class VolumetricCloudsPass : public PostprocessPass {
public:
    explicit VolumetricCloudsPass(Scene &scene)
        : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/volumetricClouds.frag"))
#ifdef _WIN32
          ,noiseGenShader(Shader("shader/compute/volumetricClouds.comp"))
#endif
          {
    }

    void GUIRender() override {
        ImGui::Checkbox("Enable ShyBox", &scene.showSkybox);
    }

    void render(RenderResource &resource) override {
        int nextFreeTextureId = bindParams(resource);
        // 参数设置
        postShader.setInt("noiseTexture", nextFreeTextureId);
        postShader.bindTexture("noiseTexture",noiseTexture,nextFreeTextureId);
        postShader.bindCubeMapTexture("environmentMap",scene.envCubemap,nextFreeTextureId + 1);

        postShader.setInt("showSkyBox", scene.showSkybox == true?1:0);
        PostprocessPass::render(resource);
        postShader.unBind();
    }

    void init(RenderResource &resource) override {
        passName = "VolumetricCloudsPass";
        PostprocessPass::init(resource);
#ifdef _WIN32
        noiseGenShader.bind();
        create3DTextureRGBA(30, 30, 30);
        glBindImageTexture(0, noiseTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        noiseGenShader.setVec3i("texSize", glm::ivec3(40, 40, 40));

        int groupX = (30 + 7) / 8;
        int groupY = (30 + 7) / 8;
        int groupZ = (30 + 7) / 8;
        glDispatchCompute(groupX, groupY, groupZ);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        noiseGenShader.unBind();
#endif

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
#ifdef _WIN32
    Shader noiseGenShader;
#endif
};
#endif //VOLUMETRICCLOUDSPASS_H
