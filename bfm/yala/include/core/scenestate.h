#if ! defined(__SCENE_STATE__)
#define __SCENE_STATE__

#include <glm/glm.hpp>

struct SceneState
{
  glm::dvec3 lightPosition;
  
  glm::dmat4 modelMatrix;

  glm::dvec3 cameraPosition;
  glm::dmat4 viewMatrix;
  glm::dmat4 projectionMatrix;
};

#endif // __SCENE_STATE__
