#ifndef TINYRENDERER_TRIANGLE_H
#define TINYRENDERER_TRIANGLE_H
#include <Eigen/Eigen>
using namespace Eigen;
class Triangle{
public:
    Eigen::Vector4f globalCoords[3];
    Eigen::Vector3f color[3];
    Eigen::Vector2f texCoords[3];
    Eigen::Vector3f normal[3];
    Eigen::Vector3f screenCoords[3];
    Triangle();

    void setGlobalCoords(int ind, Eigen::Vector4f ver);
    void setNormal(int ind, Eigen::Vector3f n);
    void setTexCoord(int ind,Eigen::Vector2f uv);
    void setScreenCoord(int ind,int width,int height);
};

#endif //TINYRENDERER_TRIANGLE_H
