#include <game/cameras/freecamera.h>
#include <glm/ext.hpp>

//------------------------------------------------------------

FreeCamera::FreeCamera()
  : speed(0),
    yaw(0), pitch(0), roll(0),
    motion(glm::vec3(1)),
    transform(glm::dmat4(1)),
    frame(0)
{
}

//------------------------------------------------------------

FreeCamera::~FreeCamera()
{
}

//------------------------------------------------------------

void FreeCamera::Update(float elapsedMS)
{
  // Make sure the various direction vectors don't drift from unit length but spread
  // the square root(s) over a few frames...
  switch (frame % 3)
  {
  case 1: forward = glm::normalize(forward); break;
  case 2: right = glm::normalize(right); break;
  case 3: up = glm::normalize(up); break;
  default: break;
  }
  ++frame;

  // Rotate in local coordinate frame:
  transform *= glm::yawPitchRoll(yaw, pitch, roll);
  forward = glm::dvec3(transform * glm::dvec4(0,0,1,0));
  right = glm::dvec3(transform * glm::dvec4(1,0,0,0));
  up = glm::dvec3(transform * glm::dvec4(0,1,0,0));

  // Update position:
  position += elapsedMS * speed * motion;
  target = position + forward;

  // Reset motion:
  motion = glm::dvec3(0);
  yaw = pitch = roll = 0;

  // Call base class:
  Camera::Update(elapsedMS);
}
