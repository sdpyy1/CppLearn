//
// Created by liuzhuohao on 25-7-18.
//

#ifndef PRECOMPUTER_H
#define PRECOMPUTER_H
#include "../core/Shader.h"
#include "glad/glad.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

class Scene;

class preComputer {
public:

    Scene & scene;
    explicit preComputer(Scene & scene):scene(scene){};
    GLuint computeIrradianceMap();
    GLuint computePrefilterMap();
    GLuint computeLutMap();
};



#endif //PRECOMPUTER_H
