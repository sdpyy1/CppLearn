#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <Eigen/Eigen>
#include "Triangle.h"
class Model {
private:
    std::vector<Eigen::Vector3f> verts_;
    std::vector<std::vector<int> > faces_;
public:
    Model(const char *filename);
    ~Model();
    std::vector<Triangle> *triangles;
};

#endif //__MODEL_H__