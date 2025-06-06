//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_BOUNDS3_H
#define RAYTRACING_BOUNDS3_H
#include "Ray.hpp"
#include "Vector.hpp"
#include <limits>
#include <array>

class Bounds3
{
  public:
    Vector3f pMin, pMax; // two points to specify the bounding box
    Bounds3()
    {
        double minNum = std::numeric_limits<double>::lowest();
        double maxNum = std::numeric_limits<double>::max();
        pMax = Vector3f(minNum, minNum, minNum);
        pMin = Vector3f(maxNum, maxNum, maxNum);
    }
    Bounds3(const Vector3f p) : pMin(p), pMax(p) {}
    Bounds3(const Vector3f p1, const Vector3f p2)
    {
        pMin = Vector3f(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
        pMax = Vector3f(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
    }

    Vector3f Diagonal() const { return pMax - pMin; }
    int maxExtent() const
    {
        Vector3f d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    double SurfaceArea() const
    {
        Vector3f d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    Vector3f Centroid() { return 0.5 * pMin + 0.5 * pMax; }
    Bounds3 Intersect(const Bounds3& b)
    {
        return Bounds3(Vector3f(fmax(pMin.x, b.pMin.x), fmax(pMin.y, b.pMin.y),
                                fmax(pMin.z, b.pMin.z)),
                       Vector3f(fmin(pMax.x, b.pMax.x), fmin(pMax.y, b.pMax.y),
                                fmin(pMax.z, b.pMax.z)));
    }

    Vector3f Offset(const Vector3f& p) const
    {
        Vector3f o = p - pMin;
        if (pMax.x > pMin.x)
            o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y)
            o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z)
            o.z /= pMax.z - pMin.z;
        return o;
    }

    bool Overlaps(const Bounds3& b1, const Bounds3& b2)
    {
        bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
        bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
        bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
        return (x && y && z);
    }

    bool Inside(const Vector3f& p, const Bounds3& b)
    {
        return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
                p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
    }
    inline const Vector3f& operator[](int i) const
    {
        return (i == 0) ? pMin : pMax;
    }

    inline bool IntersectP(const Ray& ray, const Vector3f& invDir,
                           const std::array<int, 3>& dirisNeg) const;
};


// 这个方法有点垃圾
//inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir,
//                                const std::array<int, 3>& dirIsNeg) const
//{
//    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
//    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
//    // TODO test if ray bound intersects
//    // xyz方向的法向量
//    Vector3f nX(1,0,0);
//    Vector3f nY(0,1,0);
//    Vector3f nZ(0,0,1);
//    // x方向两个t
//    float tx1 = dotProduct(pMin-ray.origin,nX)/ dotProduct(ray.direction,nX);
//    float tx2 = dotProduct(pMax-ray.origin,nX)/ dotProduct(ray.direction,nX);
//    // y方向两个t
//    float ty1 = dotProduct(pMin-ray.origin,nY)/ dotProduct(ray.direction,nY);
//    float ty2 = dotProduct(pMax-ray.origin,nY)/ dotProduct(ray.direction,nY);
//    // z方向两个t
//    float tz1 = dotProduct(pMin-ray.origin,nZ)/ dotProduct(ray.direction,nZ);
//    float tz2 = dotProduct(pMax-ray.origin,nZ)/ dotProduct(ray.direction,nZ);
//    // 保证1<2
//    if (tx1 > tx2){
//        float temp = tx1;
//        tx1 = tx2;
//        tx2 = temp;
//    }
//    if (ty1 > ty2){
//        float temp = ty1;
//        ty1 = ty2;
//        ty2 = temp;
//    }
//    if (tz1 > tz2){
//        float temp = tz1;
//        tz1 = tz2;
//        tz2 = temp;
//    }
//    float maxEnter = std::max(tx1,std::max(ty1,tz1));
//    float minExit = std::min(tx2,std::min(ty2,tz2));
//    return minExit > 0 && maxEnter <= minExit;
//}
inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir,
                                const std::array<int, 3>& dirIsNeg) const
{
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // 计算进入x,y,z截面的最早和最晚时间

    float t_min_x = (pMin.x - ray.origin.x) * invDir[0];
    float t_min_y = (pMin.y - ray.origin.y) * invDir[1];
    float t_min_z = (pMin.z - ray.origin.z) * invDir[2];
    float t_max_x = (pMax.x - ray.origin.x) * invDir[0];
    float t_max_y = (pMax.y - ray.origin.y) * invDir[1];
    float t_max_z = (pMax.z - ray.origin.z) * invDir[2];
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
    // 如果方向是负的，就交换最早和最晚时间

    if (!dirIsNeg[0])
    {
        float t = t_min_x;
        t_min_x = t_max_x;
        t_max_x = t;
    }

    if (!dirIsNeg[1])
    {
        float t = t_min_y;
        t_min_y = t_max_y;
        t_max_y = t;
    }

    if (!dirIsNeg[2])
    {
        float t = t_min_z;
        t_min_z = t_max_z;
        t_max_z = t;
    }

    // 小小取其大，大大取其小
    float t_enter = std::max(t_min_x, std::max(t_min_y, t_min_z));
    float t_exit = std::min(t_max_x, std::min(t_max_y, t_max_z));

    // TODO test if ray bound intersects
    // 检测包围盒是否存在
    if (t_exit>=0&&t_enter<t_exit)
    {
        return true;
    }
    else
    {
        return false;
    }
}
inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2)
{
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b1.pMin, b2.pMin);
    ret.pMax = Vector3f::Max(b1.pMax, b2.pMax);
    return ret;
}

inline Bounds3 Union(const Bounds3& b, const Vector3f& p)
{
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b.pMin, p);
    ret.pMax = Vector3f::Max(b.pMax, p);
    return ret;
}

#endif // RAYTRACING_BOUNDS3_H
