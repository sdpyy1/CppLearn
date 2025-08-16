//
// Created by Administrator on 2025/7/25.
//

#ifndef OPENGLRENDER_RENDERPIPELINE_H
#define OPENGLRENDER_RENDERPIPELINE_H


#include <vector>
#include <memory>
#include "pass/RenderPass.h"
#include "pass/ShadowPass.h"
#include "pass/GeometryPass.h"
#include "pass/LightingPass.h"
#include "pass/DebugPass.h"
#include "pass/postprocess/PostprocessPass.h"
#include "core/Shader.h"

#include "pass/postprocess/PostProcessManager.h"

class RenderPipeline {
public:
    std::unique_ptr<PostProcessManager> postProcessManager = nullptr;

    void addPass(std::unique_ptr<RenderPass> pass) {
        passes.push_back(std::move(pass));
    }

    void init(Scene &scene) {
        createDefaultResources();
        for (const auto & pass: passes) {
            pass->init(resource);
        }
    }

    void createDefaultResources() {
        // 正方形VAO，用于铺满屏幕渲染
        resource.VAOs["quad"] = createFullscreenTriangleVAO(); // 用一个三角形就拿下
        // debug纹理专用shader
        resource.shaders["quadShader"] = std::make_unique<Shader>("shader/quad.vert", "shader/quad.frag");
    }

    void finalOutput() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);// 因为我在最后绘制时写入了深度（为了天空盒），但是很多地方深度是0，他颜色值会消失，所以这里要直接全部通过测试，但写入深度
        finalShader.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // 清除纹理
        finalShader.bindTexture("preTexture",resource.textures["preTexture"],0);
        finalShader.bindTexture("gDepth",resource.textures["gDepth"],1);

        glBindVertexArray(resource.VAOs["quad"]);
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
        glBindVertexArray(0);
        finalShader.unBind();
    }

    void render() {
        for (auto &pass: passes) {
            if (pass->passName == "DebugPass") {
                // 先把最终画面输出了，再输出Debug信息
                finalOutput();
            }
            GL_CALL(pass->render(resource));
        }
    }

    RenderResource &getResources() { return resource; }

    void setupDeferredRenderPipeline(Scene &scene) {
        addPass(std::make_unique<ShadowPass>(scene));
        addPass(std::make_unique<GeometryPass>(scene));
        addPass(std::make_unique<LightingPass>(scene));

        // 装载后处理Pass
        postProcessManager = PostProcessManager::defaultPostProcess(scene, resource);
        for (const auto postprocessPass: postProcessManager->passes) {
            addPass(std::unique_ptr<PostprocessPass>(postprocessPass));
        }

        // DebugPass 画在最前边
        addPass(std::make_unique<DebugPass>(scene));
        init(scene);
    }


    // 方形VAO
    static GLuint createQuadVAO() {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };
        GLuint quadVAO;
        GLuint quadVBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

        glEnableVertexAttribArray(1); // texCoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

        glBindVertexArray(0);
        return quadVAO;
    }

    static GLuint createFullscreenTriangleVAO() {
        // 3 个顶点（NDC 坐标 + UV）
        float triangleVertices[] = {
            // positions   // texCoords
            -1.0f, -1.0f, 0.0f, 0.0f, // 左下
            3.0f, -1.0f, 2.0f, 0.0f, // 右下(超出屏幕)
            -1.0f, 3.0f, 0.0f, 2.0f // 左上(超出屏幕)
        };

        GLuint triVAO, triVBO;
        glGenVertexArrays(1, &triVAO);
        glGenBuffers(1, &triVBO);

        glBindVertexArray(triVAO);
        glBindBuffer(GL_ARRAY_BUFFER, triVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

        glEnableVertexAttribArray(1); // texCoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

        glBindVertexArray(0);
        return triVAO;
    }

private:
    // unique_ptr表示第一无二的指针，也就是这些Pass实例，不能被其他unique_ptr指向 （一个对象只能被一个unique_ptr指）
    std::vector<std::unique_ptr<RenderPass> > passes;
    // 管理pipeline上的资源
    RenderResource resource;

    Shader finalShader = Shader("shader/quad.vert", "shader/screen.frag");

};


#endif //OPENGLRENDER_RENDERPIPELINE_H
