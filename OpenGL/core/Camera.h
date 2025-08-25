#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认相机参数
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 8.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float NEAR = 0.1f;
const float FAR = 1000.f;

// TODO：基于四元数的相机系统、轨迹球系统
class Camera
{
public:
    int width;
    int height;
    glm::vec3 Position;
    float Zoom;
    float Near;
    float Far;
    Camera(int window_w, int window_h,
           glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH, float Near = NEAR, float Far = FAR);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float x_offset, float y_offset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float y_offset);

private:
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;

    void updateCameraVectors(); // 更新方向向量
};
