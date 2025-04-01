#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <Eigen/Eigen>
#include "Triangle.h"
#include "Texture.h"
class Model {
private:
public:
    Model(const char * objFileName,const char * texFileName);

    void setModelTransformation(float angleX, float angleY, float angleZ, float tx, float ty, float tz, float sx, float sy, float sz);
    void setViewTransformation(Eigen::Vector3f eye_pos, Eigen::Vector3f target, Eigen::Vector3f up);
    void setProjectionTransformation(float fovY, float aspectRatio, float near, float far);
    Matrix4f getMVP();
    ~Model();

    Matrix4f modelMatrix;
    Matrix4f viewMatrix;
    Matrix4f projectionMatrix;

    std::vector<Triangle> triangleList;
    Texture texture;
};

#endif //__MODEL_H__