#include <camera.h>
#include <glm/ext.hpp>


Camera::Camera()
{
}

void Camera::Initialise(size_t screenWidth, size_t screenHeight)
{
  displayResolutionX = screenWidth;

  aspectRatio = (float)screenWidth / (float)screenHeight;
  fov = 45.0;
  nearClip = 1.0;
  farClip = 1000.0;

  yaw = 0;
  pitch = 0;
  roll = 0;
  speed = 5;

  movement = glm::vec3(0);
  position = glm::vec3(0);

  up = glm::vec3(0,1,0);
  forward = glm::vec3(0,0,-1);
  right = glm::vec3(1,0,0);

  viewMatrix = glm::mat4(1);
  projectionMatrix = glm::mat4(1);
  viewProjectionMatrix = glm::mat4(1);
}

void Camera::Update(float elapsedMS)
{
  // Determine where the local XYZ axes are pointing...
  const glm::mat4 orientation = glm::yawPitchRoll(yaw, pitch, roll);

  // Convert that into a world-space transform by taking the transpose of the inverse...
  const glm::mat4 invOrientation = glm::transpose(glm::inverse(orientation));

  // Define new local axes...
  up = (glm::vec3)(invOrientation * glm::vec4(0,1,0,0));
  forward = (glm::vec3)(invOrientation * glm::vec4(0,0,-1,0));
  right = (glm::vec3)(invOrientation * glm::vec4(1,0,0,0));

  // Move along the local XYZ axes...
  const float elapsedSeconds = elapsedMS / 1000.0f;
  const float velocity = elapsedSeconds * speed;
  position += (glm::vec3)(velocity * glm::vec4(movement,0) * orientation);
  normalisedPosition = glm::normalize(position);

  // Compute new matrices...
  viewMatrix = orientation * glm::translate(-position);
  projectionMatrix = glm::perspective(fov, aspectRatio, nearClip, farClip);
  viewProjectionMatrix = projectionMatrix * viewMatrix;

  // Compute this every frame so that the field of view can be dynamic:
  twoTanThetaover2 = float(2 * tan(fov / 2));

  movement = glm::vec3(0);
}
