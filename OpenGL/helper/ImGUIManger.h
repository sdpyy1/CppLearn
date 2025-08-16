//
// Created by Administrator on 2025/7/23.
//

#ifndef OPENGLRENDER_IMGUIMANGER_H
#define OPENGLRENDER_IMGUIMANGER_H


#include "Scene.h"
#include "../RenderPipeline.h"

class ImGUIManger {
public:
    Scene &scene;
    RenderPipeline * renderPipeline;
    explicit ImGUIManger(Scene &scene,RenderPipeline * renderPipeline):scene(scene),renderPipeline(renderPipeline){}
    void render();

    void renderModelTransformUI() const;

    void renderModelListUI();

    void renderLightListUI();

    void renderAddModelUI();

    void renderIBLSelectionUI();

    void renderLightProperties();

    void renderShadowSetting();

    void renderDebugTextureSelector();

    void renderPostprocessSetting() const;
};


#endif //OPENGLRENDER_IMGUIMANGER_H
