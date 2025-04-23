#ifndef PLANE_H
#define PLANE_H
#include<iostream>
#include<utils.h>
class Plane
{
public:
    Plane(const std::string path);
    unsigned int texture, planeVAO, planeVBO;
    void render();

private:
    float planeVertices[48] = {
        25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
        -25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f
    };
};

#endif // PLANE_H
