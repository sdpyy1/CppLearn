////
//// Created by Administrator on 2025/7/14.
////
//
//#ifndef OPENGLRENDER_LIGHTINGPASS_CPP
//#define OPENGLRENDER_LIGHTINGPASS_CPP
//
//#include "RenderPass.h"
//#include "../core/Shader.h"
//
//class LightingPass : public RenderPass {
//public:
//    Shader lightingShader;
//    GLuint gPosition, gNormal, gAlbedoSpec;
//
//    void Init() override {
//        lightingShader = Shader("lighting.vs", "lighting.fs");
//    }
//
//    void SetGBufferTextures(GLuint pos, GLuint norm, GLuint alb) {
//        gPosition = pos; gNormal = norm; gAlbedoSpec = alb;
//    }
//
//    void Render() override {
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        lightingShader.use();
//
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, gPosition);
//        lightingShader.setInt("gPosition", 0);
//
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, gNormal);
//        lightingShader.setInt("gNormal", 1);
//
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
//        lightingShader.setInt("gAlbedoSpec", 2);
//
//        RenderQuad(); // 渲染全屏 Quad
//    }
//};
//
//#endif //OPENGLRENDER_LIGHTINGPASS_CPP
