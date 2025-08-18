#ifndef VOLUMETRICCLOUDSPASS_H
#define VOLUMETRICCLOUDSPASS_H
#include "PostprocessPass.h"
#include <vector>
#include <random>
#include <fstream>
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
        postShader.bind3DTexture("noiseTexture",noiseTexture,nextFreeTextureId++);
        postShader.bindCubeMapTexture("environmentMap",scene.envCubemap,nextFreeTextureId++);
        postShader.bindTexture("weatherTexture",weatherTexture,nextFreeTextureId++);

        postShader.setInt("showSkyBox", scene.showSkybox == true?1:0);
        GL_CALL(PostprocessPass::render(resource));
        postShader.unBind();
    }

    void init(RenderResource &resource) override {
        passName = "VolumetricCloudsPass";
        PostprocessPass::init(resource);
        int TEX_SIZE_X = 128;
        int TEX_SIZE_Y = 128;
        int TEX_SIZE_Z = 128;

#ifdef _WIN32
        noiseGenShader.bind();
        create3DTextureRGBA(TEX_SIZE_X, TEX_SIZE_Y, TEX_SIZE_Z);
        glBindImageTexture(0, noiseTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        // 调用 compute shader
        GLuint groupX = (TEX_SIZE_X + 7) / 8;
        GLuint groupY = (TEX_SIZE_Y + 7) / 8;
        GLuint groupZ = (TEX_SIZE_Z + 0); // local_size_z=1
        glDispatchCompute(groupX, groupY, groupZ);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        noiseGenShader.unBind();
        // 读取纹理数据
        std::vector<unsigned char> data(TEX_SIZE_X * TEX_SIZE_Y * TEX_SIZE_Z);
        glBindTexture(GL_TEXTURE_3D, tex3D);
        glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());

        // 保存为二进制文件
        std::ofstream file("worleyNoise3D.bin", std::ios::binary);
        file.write(reinterpret_cast<char*>(data.data()), data.size());
        file.close();
        std::cout << "3D noise texture saved as worleyNoise3D.bin\n";
#endif


#ifdef __APPLE__
        noiseTexture = Shader::load3DTextureFromFile("worleyNoise3D.bin",TEX_SIZE_X,TEX_SIZE_Y,TEX_SIZE_Z);
#endif
        weatherTexture = Shader::loadTextureFormFile("assets/cloud/weatherTexture.png");
    }

private:
    GLuint noiseTexture = 0;
    GLuint weatherTexture = 0;

#ifdef _WIN32
    Shader noiseGenShader;
#endif

};
#endif //VOLUMETRICCLOUDSPASS_H
