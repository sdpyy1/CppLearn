#pragma once
#include "../core/Shader.h"
#include "../core/camera.h"
#include "../core/VertexArray.h"
#include "../core/VertexBuffer.h"
#include "../core/IndexBuffer.h"

class DefaultSceneSetup {
public:
    DefaultSceneSetup(Camera& cam,int width, int height);
    void draw(Shader& shader);
    int width;
    int height;

private:
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ibo;
    Camera& camera;


    void setupFloorMesh();
};
