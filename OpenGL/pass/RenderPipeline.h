//
// Created by Administrator on 2025/7/25.
//

#ifndef OPENGLRENDER_RENDERPIPELINE_H
#define OPENGLRENDER_RENDERPIPELINE_H


#include <vector>
#include <memory>
#include "RenderPass.h"
#include "ShadowPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "DebugPass.h"
#include "postprocess/PostprocessPass.h"
#include "../core/Shader.h"
#include "postprocess/SSRPass.h"
#include "postprocess/FinalColorPass.h"
#include "postprocess/BloomPass.h"

class RenderPipeline {

public:
    void addPass(std::unique_ptr<RenderPass> pass) {
        passes.emplace_back(std::move(pass));
    }

    void init() {
        createDefaultResources();
        for (auto& pass : passes) {
            pass->init(resource);
        }
    }
    void createDefaultResources() {
        // 正方形VAO，用于铺满屏幕渲染
        resource.VAOs["quad"] = createQuadVAO();
        // debug纹理专用shader
        resource.shaders["quadShader"] = std::make_unique<Shader>("shader/quad.vert", "../shader/quad.frag");
    }

    void render() {
        for (auto& pass : passes) {
            pass->render(resource);
        }

    }

    RenderResource& getResources() { return resource; }

    static std::unique_ptr<RenderPipeline> setupDeferredRenderPipeline(Scene& scene) {
        auto pipeline = std::make_unique<RenderPipeline>();
        pipeline->addPass(std::make_unique<ShadowPass>(scene));
        pipeline->addPass(std::make_unique<GeometryPass>(scene));
        pipeline->addPass(std::make_unique<LightingPass>(scene));
        pipeline->addPass(std::make_unique<SSRPass>(scene));
//        pipeline->addPass(std::make_unique<BloomPass>(scene));
        pipeline->addPass(std::make_unique<FinalColorPass>(scene));
        pipeline->addPass(std::make_unique<DebugPass>(scene));
        pipeline->init();
        return pipeline;
    }


    // 方形VAO
    static GLuint createQuadVAO() {
        float quadVertices[] = {
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
        };
        GLuint quadVAO;
        GLuint quadVBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1); // texCoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
        return quadVAO;
    }

private:
    // unique_ptr表示第一无二的指针，也就是这些Pass实例，不能被其他unique_ptr指向 （一个对象只能被一个unique_ptr指）
    std::vector<std::unique_ptr<RenderPass>> passes;
    // 管理pipeline上的资源
    RenderResource resource;
};


#endif //OPENGLRENDER_RENDERPIPELINE_H
