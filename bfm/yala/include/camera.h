#if ! defined(__CAMERA__)
#define __CAMERA__

#include <glm/glm.hpp>

class Camera
{
public:
  Camera();

  void Initialise(size_t screenWidth, size_t screenHeight);
  void Update(float elapsedMS);

  // Used to determine the view frustum width at a real-world distance D:
  //    w = 2 * D * tan(fov / 2)
  // where fov is the field-of-view angle and D is the distance to a point.
  float twoTanThetaover2;

  size_t displayResolutionX;

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
  glm::vec3 normalisedPosition;
  glm::vec3 up;
  glm::vec3 forward;
  glm::vec3 right;
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::mat4 viewProjectionMatrix;
};

#endif // __CAMERA__
