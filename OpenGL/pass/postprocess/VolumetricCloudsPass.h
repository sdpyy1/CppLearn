#ifndef VOLUMETRICCLOUDSPASS_H
#define VOLUMETRICCLOUDSPASS_H
#include "PostprocessPass.h"
#include <vector>
#include <random>
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
        postShader.bind3DTexture("noiseTexture",noiseTexture,nextFreeTextureId);
        postShader.bindCubeMapTexture("environmentMap",scene.envCubemap,nextFreeTextureId + 1);

        postShader.setInt("showSkyBox", scene.showSkybox == true?1:0);
        GL_CALL(PostprocessPass::render(resource));
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
#ifdef __APPLE__
        noiseTexture = create3DNoiseTexture(32);
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
    static GLuint create3DNoiseTexture(int size) {
        // 1. 分配噪声数据
        std::vector<float> data(size * size * size);

        std::mt19937 rng(1234); // 固定种子，便于复现
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (int z = 0; z < size; ++z) {
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    int idx = x + y * size + z * size * size;
                    data[idx] = dist(rng); // [0,1] 随机值
                }
            }
        }

        // 2. 创建 OpenGL 3D 纹理
        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_3D, texID);

        glTexImage3D(
            GL_TEXTURE_3D,
            0,                // mipmap level
            GL_R8,            // 内部存储格式 (单通道 8bit)
            size, size, size,
            0,                // border
            GL_RED,           // 上传数据通道
            GL_FLOAT,         // 数据类型
            data.data()       // 数据指针
        );

        // 3. 设置采样参数
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glBindTexture(GL_TEXTURE_3D, 0);

        return texID;
    }
#ifdef _WIN32
    Shader noiseGenShader;
#endif
};
#endif //VOLUMETRICCLOUDSPASS_H
