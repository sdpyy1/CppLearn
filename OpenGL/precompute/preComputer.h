//
// Created by liuzhuohao on 25-7-18.
//

#ifndef PRECOMPUTER_H
#define PRECOMPUTER_H
#include "../core/Shader.h"
#include "glad/glad.h"

class Scene;

class preComputer {
public:
    Scene & scene;
    explicit preComputer(Scene & scene):scene(scene){};
    GLuint computeIrradianceMap(GLuint envCubemap);
};



#endif //PRECOMPUTER_H
