#include <glad/glad.h>
#include "RenderPass.h"
#include "../core/Shader.h"
#include "../core/WindowManager.h"
#include "../helper/scene.h"

class GeometryPass : public RenderPass {
private:
    Shader shader;
    Shader debugShader;
    GLuint debugVAO;
    Scene scene;
    int width, height;
    GLuint gBuffer;
public:
    GLuint gPosition, gNormal, gAlbedo,gMaterial;
    GeometryPass(Scene &scene): scene(scene), shader("shader/geometry.vert", "shader/geometry.frag"),debugShader("shader/fullscreen_debug.vert", "shader/fullscreen_debug.frag")
    {
        width = scene.width;
        height = scene.height;
    }

    void init() override{
        // 初始化 G-Buffer、纹理附件、shader
        InitGBuffer(); // 生成 FBO.并且挂载附件
        initDebug();
    }
    void InitGBuffer() {
        GL_CALL(glGenFramebuffers(1, &gBuffer));
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));

        // 1. Position (RGB16F)
        GL_CALL(glGenTextures(1, &gPosition));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gPosition));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0));

        // 2. Normal (RGB16F)
        GL_CALL(glGenTextures(1, &gNormal));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gNormal));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0));

        // 3. Albedo (RGB8 or RGBA8)
        GL_CALL(glGenTextures(1, &gAlbedo));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gAlbedo));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0));

        // 4. Material info: (R:Metallic, G:Roughness, B:AO, A:Emission strength)
        GL_CALL(glGenTextures(1, &gMaterial));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gMaterial));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMaterial, 0));

        // 5. Depth Buffer (renderbuffer)
        GLuint rboDepth;
        GL_CALL(glGenRenderbuffers(1, &rboDepth));
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, rboDepth));
        GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
        GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth));

        // 6. 设置 draw buffers
        GLuint attachments[4] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3
        };
        glDrawBuffers(4, attachments);

        // 7. 检查完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Framebuffer not complete!" << std::endl;
        cout << "G-Buffer init!"<<endl;
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void render() override {
        GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.drawAll(shader);
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void initDebug(){
        float quadVertices[] = {
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
        };
        GLuint quadVBO;
        glGenVertexArrays(1, &debugVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(debugVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);
    }

    void debug() {
        debugShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GL_CALL(glDisable(GL_DEPTH_TEST));
        // 每个角落一个 viewport（设为四分之一）
        int w = width / 2;
        int h = height / 2;

        GL_CALL(glBindVertexArray(debugVAO));

        // 左上 - Position
        GL_CALL(glViewport(0, h, w, h));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gPosition));
        debugShader.setInt("gBufferTexture", 0);
        debugShader.setInt("visualizeMode", 0);
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

        // 右上 - Normal
        GL_CALL(glViewport(w, h, w, h));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gNormal));
        debugShader.setInt("visualizeMode", 1);
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

        // 左下 - Albedo
        GL_CALL(glViewport(0, 0, w, h));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gAlbedo));
        debugShader.setInt("visualizeMode", 0);
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

        // 右下 - Material
        GL_CALL(glViewport(w, 0, w, h));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gMaterial));
        debugShader.setInt("visualizeMode", 0); // 如需可视化 metallic/ao 可额外处理
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

        // 恢复全屏 viewport
        GL_CALL(glBindVertexArray(0));
        GL_CALL(glViewport(0, 0, width, height));
    }

};
