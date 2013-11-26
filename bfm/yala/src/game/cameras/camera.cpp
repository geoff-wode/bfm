#include <game/cameras/camera.h>
#include <glm/ext.hpp>

//------------------------------------------------------------

Camera::Camera()
  : nearClip(1),
    farClip(100000000),
    fieldOfViewAngle(45),
    position(glm::dvec3(0)),
    target(glm::dvec3(0, 0, -1)),
    forward(glm::dvec3(0, 0, 1)),
    right(glm::dvec3(1, 0, 0)),
    up(glm::dvec3(0, 1, 0)),
    viewMatrix(glm::dmat4(1)),
    projectionMatrix(glm::dmat4(1))
{
}

//------------------------------------------------------------

Camera::~Camera()
{
}

//------------------------------------------------------------

void Camera::Update(float elapsedMS)
{
  viewMatrix = glm::lookAt(position, target, up);

  projectionMatrix = glm::perspective(fieldOfViewAngle, aspectRatio, nearClip, farClip);
}
