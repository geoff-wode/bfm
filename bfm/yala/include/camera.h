#if ! defined(__CAMERA__)
#define __CAMERA__

#include <glm/glm.hpp>

class Camera
{
public:
  Camera(size_t screenWidth, size_t screenHeight);
  void Update(float elapsedMS);

  float fov;
  float tanFoVOver2;          // == tan(field_of_view_angle / 2)
  size_t displayResolutionX;  // number of pixels in X
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

#endif // __CAMERA__
