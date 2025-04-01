#include "Triangle.h"

Triangle::Triangle() {
    globalCoords[0] << 0,0,0,1;
    globalCoords[1] << 0,0,0,1;
    globalCoords[2] << 0,0,0,1;

    color[0] << 0.0, 0.0, 0.0;
    color[1] << 0.0, 0.0, 0.0;
    color[2] << 0.0, 0.0, 0.0;

    texCoords[0] << 0.0, 0.0;
    texCoords[1] << 0.0, 0.0;
    texCoords[2] << 0.0, 0.0;
}

void Triangle::setGlobalCoord(int ind, Vector4f ver){
    globalCoords[ind] = ver;
}
void Triangle::setNormal(int ind, Vector3f n){
    normal[ind] = n;
}
void Triangle::setTexCoord(int ind, Vector2f uv) {
    texCoords[ind] = uv;
}
void Triangle::setScreenCoords(const Eigen::Matrix4f& transMatrix){
    for (int i = 0; i < 3; ++i){
        screenCoords[i] = transMatrix*globalCoords[i];
        screenCoords[i].x() /= screenCoords[i].w();
        screenCoords[i].y() /= screenCoords[i].w();
        screenCoords[i].z() /= screenCoords[i].w();
    }
}
