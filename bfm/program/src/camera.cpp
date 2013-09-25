#include <camera.h>
#include <glm/ext.hpp>


void CameraInit(Camera& camera)
{
  camera.fov = 45.0f;
  camera.nearClip = 0.1f;
  camera.farClip = 1000.0f;
  camera.yaw = 0;
  camera.pitch = 0;
  camera.roll = 0;
  camera.speed = 5;
  camera.movement = glm::vec3(0);
  camera.position = glm::vec3(0);
  camera.up = glm::vec3(0,1,0);
  camera.forward = glm::vec3(0,0,-1);
  camera.right = glm::vec3(1,0,0);
  camera.viewMatrix = glm::mat4();
  camera.projectionMatrix = glm::mat4();
}

void CameraUpdate(float elapsedMS, Camera& camera)
{
  // Determine where the local XYZ axes are pointing...
  const glm::mat4 orientation = glm::yawPitchRoll(camera.yaw, camera.pitch, camera.roll);
  const glm::mat4 invOrientation(glm::inverse(orientation));
  camera.up = (glm::vec3)(invOrientation * glm::vec4(0,1,0,0));
  camera.forward = (glm::vec3)(invOrientation * glm::vec4(0,0,-1,0));
  camera.right = (glm::vec3)(invOrientation * glm::vec4(1,0,0,0));

  // Move along the local XYZ axes...
  const float elapsedSeconds = elapsedMS / 1000.0f;
  const float velocity(elapsedSeconds * camera.speed);
  camera.position += (glm::vec3)(velocity * glm::vec4(camera.movement,0) * orientation);
  camera.movement = glm::vec3(0);

  // Compute new matrices...
  camera.viewMatrix = orientation * glm::translate(-camera.position);
  camera.projectionMatrix = glm::perspective(camera.fov, camera.aspectRatio, camera.nearClip, camera.farClip);
  camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
}
