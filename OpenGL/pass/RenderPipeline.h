//
// Created by Administrator on 2025/7/25.
//

#ifndef OPENGLRENDER_RENDERPIPELINE_H
#define OPENGLRENDER_RENDERPIPELINE_H


#include <vector>
#include <memory>
#include "RenderPass.h"
#include "ShadowPass.h"
#include "DebugPass.h"
#include "PostprocessPass.h"

class RenderPipeline {
    std::vector<std::unique_ptr<RenderPass>> passes;
    RenderResource resource;

public:
    void addPass(std::unique_ptr<RenderPass> pass) {
        passes.push_back(std::move(pass));
    }

    void init() {
        for (auto& pass : passes) {
            pass->init(resource);
        }
    }

    void render() {
        for (auto& pass : passes) {
            pass->render(resource);
        }

    }

    RenderResource& getResources() { return resource; }

    static std::unique_ptr<RenderPipeline> setupDefaultPipeline(Scene& scene) {
        auto pipeline = std::make_unique<RenderPipeline>();
        pipeline->addPass(std::make_unique<ShadowPass>(scene));
        pipeline->addPass(std::make_unique<GeometryPass>(scene));
        pipeline->addPass(std::make_unique<LightingPass>(scene));
        pipeline->addPass(std::make_unique<PostprocessPass>(scene));
//        pipeline->addPass(std::make_unique<DebugPass>(scene));
        pipeline->init();
        return pipeline;
    }
};


#endif //OPENGLRENDER_RENDERPIPELINE_H
