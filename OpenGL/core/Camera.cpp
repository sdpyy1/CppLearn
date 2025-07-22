#include "Camera.h"

#include <iostream>

Camera::Camera(int window_w, int window_h,
               glm::vec3 position, glm::vec3 up,
               float yaw, float pitch, float Near, float Far)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      WorldUp(up),
      Yaw(yaw),
      Pitch(pitch),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      width(window_w),
      height(window_h),
      Position(position),
      Zoom(ZOOM),
      Near(Near),
      Far(Far)
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(Zoom), static_cast<float>(width) / static_cast<float>(height), Near, Far);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float x_offset, float y_offset, GLboolean constrainPitch)
{
    x_offset *= MouseSensitivity;
    y_offset *= MouseSensitivity;

    Yaw += x_offset;
    Pitch += y_offset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float y_offset)
{
    Zoom -= y_offset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
