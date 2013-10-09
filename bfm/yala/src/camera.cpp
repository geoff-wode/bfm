#include <camera.h>
#include <glm/ext.hpp>


Camera::Camera(size_t screenWidth, size_t screenHeight)
{
  displayResolutionX = screenWidth;

  aspectRatio = (double)screenWidth / (double)screenHeight;
  fov = 45.0f;
  nearClip = 1.0f;
  farClip = 1000.0f;

  tanFoVOver2 = tan(fov * 0.5f);

  yaw = 0;
  pitch = 0;
  roll = 0;
  speed = 5;

  movement = glm::dvec3(0);
  position = glm::dvec3(0);

  up = glm::dvec3(0,1,0);
  forward = glm::dvec3(0,0,-1);
  right = glm::dvec3(1,0,0);

  viewMatrix = glm::dmat4(1);
  projectionMatrix = glm::dmat4(1);
  viewProjectionMatrix = glm::dmat4(1);
}

void Camera::Update(double elapsedMS)
{
  // Determine where the local XYZ axes are pointing...
  const glm::dmat4 orientation = glm::yawPitchRoll(yaw, pitch, roll);

  // Convert that into a world-space transform by taking the transpose of the inverse...
  const glm::dmat4 invOrientation = glm::transpose(glm::inverse(orientation));

  // Define new local axes...
  up = (glm::dvec3)(invOrientation * glm::dvec4(0,1,0,0));
  forward = (glm::dvec3)(invOrientation * glm::dvec4(0,0,-1,0));
  right = (glm::dvec3)(invOrientation * glm::dvec4(1,0,0,0));

  // Move along the local XYZ axes...
  const double elapsedSeconds = elapsedMS / 1000.0f;
  const double velocity = elapsedSeconds * speed;
  position += (glm::dvec3)(velocity * glm::dvec4(movement,0) * orientation);
  movement = glm::dvec3(0);

  // Compute new matrices...
  viewMatrix = orientation * glm::translate(-position);
  projectionMatrix = glm::perspective(fov, aspectRatio, nearClip, farClip);
  viewProjectionMatrix = projectionMatrix * viewMatrix;
}
