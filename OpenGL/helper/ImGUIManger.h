//
// Created by Administrator on 2025/7/23.
//

#ifndef OPENGLRENDER_IMGUIMANGER_H
#define OPENGLRENDER_IMGUIMANGER_H


#include "Scene.h"

class ImGUIManger {
public:
    Scene &scene;
    explicit ImGUIManger(Scene &scene):scene(scene){}
    void render();

    void renderModelTransformUI();

    void renderModelListUI();

    void renderLightListUI();

    void renderAddModelUI();

    void renderIBLSelectionUI();

    void renderLightProperties();

    void renderShadowSetting();
};


#endif //OPENGLRENDER_IMGUIMANGER_H
