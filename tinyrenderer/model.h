#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <Eigen/Eigen>
#include "Triangle.h"
#include "Texture.h"
class Model {
private:
public:
    Model(const char *objFileName,const char * texFileName);
    ~Model();
    std::vector<Triangle> triangleList;
    Texture texture;
};

#endif //__MODEL_H__