//
// Created by Administrator on 2025/8/8.
//

#ifndef AABB_H
#define AABB_H
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>
#include <limits>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

inline AABB computeAABB(const std::vector<glm::vec3>& vertices) {
    glm::vec3 minPoint( std::numeric_limits<float>::max());
    glm::vec3 maxPoint(-std::numeric_limits<float>::max());

    for(const auto& v : vertices) {
        minPoint.x = std::min(minPoint.x, v.x);
        minPoint.y = std::min(minPoint.y, v.y);
        minPoint.z = std::min(minPoint.z, v.z);

        maxPoint.x = std::max(maxPoint.x, v.x);
        maxPoint.y = std::max(maxPoint.y, v.y);
        maxPoint.z = std::max(maxPoint.z, v.z);
    }
    return {minPoint, maxPoint};
}
#endif //AABB_H
