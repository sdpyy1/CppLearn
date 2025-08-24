#ifndef VOLUMETRICCLOUDSPASS_H
#define VOLUMETRICCLOUDSPASS_H
#include "PostprocessPass.h"
#include <vector>
#include <random>
#include <fstream>
class VolumetricCloudsPass final : public PostprocessPass {
public:
    explicit VolumetricCloudsPass(Scene &scene)
        : PostprocessPass(scene, Shader("shader/quad.vert", "shader/postprocess/volumetricClouds.frag"))
          ,noiseGenShader(Shader("shader/compute/volumetricCloudsNoise.comp")){}

    void GUIRender() override {
        ImGui::Checkbox("Enable ShyBox", &scene.showSkybox);
        ImGui::SliderFloat("Cloud Feather", &scene.cloudFeather, 0.0f, 1.0f, "%.2f");

    }

    void render(RenderResource &resource) override {
        int nextFreeTextureId = bindParams(resource);
        // 参数设置
        postShader.bind3DTexture("basicNoiseTexture",basicNoiseTexture,nextFreeTextureId++);
        postShader.bind3DTexture("detailNoiseTexture",detailNoiseTexture,nextFreeTextureId++);
        postShader.bindCubeMapTexture("environmentMap",scene.envCubemap,nextFreeTextureId++);
        postShader.bindTexture("weatherTexture",weatherTexture,nextFreeTextureId++);
        postShader.bindTexture("cloudCurlNoise",cloudCurlNoise,nextFreeTextureId++);
        postShader.setInt("showSkyBox", scene.showSkybox == true?1:0);
        postShader.setFloat("time",static_cast<float>(glfwGetTime()));
        // GUi参数
        postShader.setFloat("cloudFeather",scene.cloudFeather);

        GL_CALL(PostprocessPass::render(resource));
        postShader.unBind();
    }

    void init(RenderResource &resource) override {
        passName = "VolumetricCloudsPass";
        PostprocessPass::init(resource);
        int TEX_SIZE_X = 128;
        int TEX_SIZE_Y = 128;
        int TEX_SIZE_Z = 128;
        noiseGenShader.bind();
        basicNoiseTexture = create3DTextureRGBA(TEX_SIZE_X, TEX_SIZE_Y, TEX_SIZE_Z);
        detailNoiseTexture = create3DTextureRGBA(TEX_SIZE_X, TEX_SIZE_Y, TEX_SIZE_Z);
        glBindImageTexture(0, basicNoiseTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, detailNoiseTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        // 调用 compute shader
        GLuint groupX = (TEX_SIZE_X + 7) / 8;
        GLuint groupY = (TEX_SIZE_Y + 7) / 8;
        GLuint groupZ = (TEX_SIZE_Z + 0); // local_size_z=1
        glDispatchCompute(groupX, groupY, groupZ);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        noiseGenShader.unBind();

        weatherTexture = Shader::loadTextureFormFile("assets/cloud/T_CloudWetherMap.png");
        cloudCurlNoise = Shader::loadTextureFormFile("assets/cloud/T_CurlNoise.png");
    }

private:
    GLuint basicNoiseTexture = 0;
    GLuint detailNoiseTexture = 0;
    GLuint weatherTexture = 0;
    GLuint cloudCurlNoise = 0;

    Shader noiseGenShader;
    GLuint create3DTextureRGBA(const int width, const int height, const int depth) {
        GLuint noiseTexture;
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
        return noiseTexture;
    }

};
#endif //VOLUMETRICCLOUDSPASS_H
