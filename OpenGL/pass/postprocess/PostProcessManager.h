//
// Created by Administrator on 2025/7/28.
//

#ifndef OPENGLRENDER_POSTPROCESSMANAGER_H
#define OPENGLRENDER_POSTPROCESSMANAGER_H

#include <vector>
#include <memory>
#include "PostprocessPass.h"
#include "FinalColorPass.h"
#include "BloomPass.h"
#include "SSRPass.h"

class PostProcessManager{
public:
    std::vector<PostprocessPass *> passes;
    explicit PostProcessManager(RenderResource &resource):resource(resource){};
    void addPass(PostprocessPass* newPass){
        passes.emplace_back(newPass);
    }
    void init() {
        for (auto& pass : passes) {
            pass->init(resource);
        }
    }
    void render(){
        for (auto& pass : passes) {
            pass->render(resource);
        }
    }

    static std::unique_ptr<PostProcessManager> defaultPostProcess(Scene& scene,RenderResource & resource) {
        auto postProcessManager = std::make_unique<PostProcessManager>(resource);
        postProcessManager->addPass(new SSRPass(scene));
        // postProcessManager->addPass(new BloomPass(scene));
        postProcessManager->addPass(new FinalColorPass(scene));
        return postProcessManager;
    }
private:
    RenderResource &resource;


};
#endif //OPENGLRENDER_POSTPROCESSMANAGER_H
