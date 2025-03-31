#ifndef TINYRENDERER_TRIANGLE_H
#define TINYRENDERER_TRIANGLE_H
#include <Eigen/Eigen>
// 描述一个三角形
class Triangle {
    std::vector<Eigen::Vector3f> vs;


public:
    Triangle(const std::vector<Eigen::Vector3f> &vs);
};


#endif //TINYRENDERER_TRIANGLE_H
