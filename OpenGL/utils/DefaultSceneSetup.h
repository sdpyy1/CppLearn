#pragma once
#include "../core/Shader.h"
#include "../core/Camera.h"
#include "../core/VAO.h"
#include "../core/VBO.h"
#include "../core/EBO.h"

class DefaultSceneSetup {
public:
    DefaultSceneSetup(Camera& cam,int width, int height);
    void draw(Shader& shader);
    int width;
    int height;

private:
    VAO vao;
    VBO vbo;
    EBO ibo;
    Camera& camera;


    void setupFloorMesh();
};
