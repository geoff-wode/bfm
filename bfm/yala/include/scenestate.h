#if ! defined(__SCENE_STATE__)
#define __SCENE_STATE__

#include <glm/glm.hpp>
#include <camera.h>
#include <device.h>

struct SceneState
{
  Device* device;
  Camera* camera;
};

#endif // __SCENE_STATE__
