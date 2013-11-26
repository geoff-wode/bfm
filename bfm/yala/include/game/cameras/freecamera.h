#if ! defined(__FREE_CAMERA__)
#define __FREE_CAMERA__

#include <game/cameras/camera.h>

class FreeCamera : public Camera
{
public:
  FreeCamera();
  virtual ~FreeCamera();

  virtual void Update(float elapsedMS);

  virtual void MoveForward() { motion += forward; }
  virtual void MoveBackward() { motion -= forward; }
  virtual void MoveRight() { motion += right; }
  virtual void MoveLeft() { motion -= right; }
  virtual void MoveUp() { motion += up; }
  virtual void MoveDown() { motion -= up; }

  double speed;
  double yaw;
  double pitch;
  double roll;

  glm::dvec3 motion;
  glm::dmat4 transform;

private:
  unsigned int frame;
};

#endif // __FREE_CAMERA__
