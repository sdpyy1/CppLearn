#pragma once

#include "Vector.hpp"
#include "global.hpp"

class Object
{
public:
    Object()
        : materialType(DIFFUSE_AND_GLOSSY)
        , ior(1.3)
        , Kd(0.8)
        , Ks(0.2)
        , diffuseColor(0.2)
        , specularExponent(25)
    {}
    virtual ~Object() = default;
    // 光线与Object是否有交点
    virtual bool intersect(const Vector3f&, const Vector3f&, float&, uint32_t&, Vector2f&) const = 0;
    // 负责计算交点处的表面属性
    virtual void getSurfaceProperties(const Vector3f&, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f&,
                                      Vector2f&) const = 0;
    // 返回Object的漫反射颜色
    virtual Vector3f evalDiffuseColor(const Vector2f&) const
    {
        return diffuseColor;
    }
    // material properties
    MaterialType materialType;
    // 折射率
    float ior;
    // 漫反射和高光反射系数
    float Kd, Ks;
    // 漫反射颜色
    Vector3f diffuseColor;
    // 高光指数（Phong模型参数），控制高光区域的大小和锐利程度（值越大，高光越集中）‌
    float specularExponent;
};
