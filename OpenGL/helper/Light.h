//
// Created by Administrator on 2025/7/14.
//

#ifndef OPENGLRENDER_LIGHT_H
#define OPENGLRENDER_LIGHT_H
#include <glm/glm.hpp>

enum class LightType
{
    Directional,
    Point,
    Spot
};

#include <glm/glm.hpp>

// 光源基类
class Light
{
public:
    glm::vec3 position;
    glm::vec3 color; // 光颜色，范围0~1
    float intensity; // 光强度 >= 0

    explicit Light(const glm::vec3& c = glm::vec3(1.0f), glm::vec3 position = glm::vec3(1.0f),float i = 1.0f)
        : color(c), intensity(i), position(position)
    {
    }

    virtual ~Light() = default;

    // 获取最终光照颜色，供shader使用
    glm::vec3 getFinalColor() const
    {
        return color * intensity;
    }

    // 纯虚函数，更新光源状态（可以用来更新矩阵、方向等）
    virtual void update() = 0;
};


// 方向光，方向固定且无衰减
class DirectionalLight : public Light
{
public:
    glm::vec3 direction; // 光方向，单位向量

    DirectionalLight(const glm::vec3& dir = glm::vec3(-0.2f, -1.0f, -0.3f),
                     const glm::vec3& c = glm::vec3(1.0f),
                     float i = 1.0f)
        : Light(c, dir,i), direction(glm::normalize(dir))
    {
    }

    void update() override
    {
        // 如果需要，可以在这里动态更新方向或者其他状态
    }
};


// 点光源，有位置且会有距离衰减
class PointLight : public Light
{
public:
    float constant; // 常数项衰减
    float linear; // 线性衰减
    float quadratic; // 二次衰减

    PointLight(const glm::vec3& pos = glm::vec3(0.0f),
               const glm::vec3& c = glm::vec3(1.0f),
               float i = 1.0f,
               float constantAtt = 1.0f,
               float linearAtt = 0.09f,
               float quadraticAtt = 0.032f)
        : Light(c, pos, i),
          constant(constantAtt), linear(linearAtt), quadratic(quadraticAtt)
    {
    }

    void update() override
    {
        // 可以用来更新位置或者动画等
    }
};

class SpotLight : public Light
{
public:
    glm::vec3 direction;
    float cutOff; // 内截锥角的余弦值 (cosine of inner cone angle)
    float outerCutOff; // 外截锥角的余弦值 (cosine of outer cone angle)

    float constant;
    float linear;
    float quadratic;

    SpotLight(const glm::vec3& pos = glm::vec3(0.0f),
              const glm::vec3& dir = glm::vec3(0.0f, -1.0f, 0.0f),
              float cutOffAngleDeg = 12.5f,
              float outerCutOffAngleDeg = 17.5f,
              const glm::vec3& c = glm::vec3(1.0f),
              float i = 1.0f,
              float constantAtt = 1.0f,
              float linearAtt = 0.09f,
              float quadraticAtt = 0.032f)
        : Light(c, pos,i),
          direction(glm::normalize(dir)),
          constant(constantAtt),
          linear(linearAtt),
          quadratic(quadraticAtt)
    {
        cutOff = glm::cos(glm::radians(cutOffAngleDeg));
        outerCutOff = glm::cos(glm::radians(outerCutOffAngleDeg));
    }

    void update() override
    {
        // 可以动态更新位置、方向等
    }
};


#endif //OPENGLRENDER_LIGHT_H
