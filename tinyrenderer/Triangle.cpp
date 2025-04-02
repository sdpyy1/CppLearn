#include "Triangle.h"
#include "iostream"
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

void Triangle::setScreenCoords(const Eigen::Matrix4f& transMatrix,int width,int height){
    for (int i = 0; i < 3; ++i){
//        std::cout << "FIRST : " << globalCoords[i].transpose() << std::endl;
        screenCoords[i] = transMatrix*globalCoords[i];
//        std::cout << "MVP after" << screenCoords[i].transpose() << std::endl;
        screenCoords[i].x() /= screenCoords[i].w();
        screenCoords[i].y() /= screenCoords[i].w();
        screenCoords[i].z() /= screenCoords[i].w();
//        std::cout << "/W after" << screenCoords[i].transpose() << std::endl;
        screenCoords[i].x() = 0.5*width*(screenCoords[i].x()+1);
        screenCoords[i].y() = 0.5*height*(screenCoords[i].y()+1);
//        std::cout << "SCREEN" << screenCoords[i].transpose() << std::endl;

    }
}
