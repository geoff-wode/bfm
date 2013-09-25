#if ! defined(__CAMERA__)
#define __CAMERA__

#include <glm/glm.hpp>


struct Camera
{
  float fov;
  float aspectRatio;
  float nearClip;
  float farClip;
  float yaw;
  float pitch;
  float roll;
  float speed;
  glm::vec3 movement;
  glm::vec3 position;
  glm::vec3 up;
  glm::vec3 forward;
  glm::vec3 right;
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::mat4 viewProjectionMatrix;
};

void CameraInit(Camera& camera);
void CameraUpdate(float elapsedMS, Camera& camera);

#endif // __CAMERA__
