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

void Triangle::setGlobalCoords(int ind, Vector4f ver){
    globalCoords[ind] = ver;
}
void Triangle::setNormal(int ind, Vector3f n){
    normal[ind] = n;
}
void Triangle::setTexCoord(int ind, Vector2f uv) {
    texCoords[ind] = uv;
}
// 简单实现正交投影
Vector3f world2screen(Vector4f globalCoord,int width,int height) {
    return Vector3f(int((globalCoord.x()+1.)*width/2.+.5), int((globalCoord.y()+1.)*height/2.+.5), globalCoord.z());
}
void Triangle::setScreenCoord(int ind,int width,int height) {
    screenCoords[ind] = world2screen(this->globalCoords[ind],width,height);
}
