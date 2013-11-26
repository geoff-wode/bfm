#if ! defined(__PLANET__)
#define __PLANET__

#include <glm/glm.hpp>
#include <boost/scoped_ptr.hpp>
#include <core/context.h>
#include <game/cameras/camera.h>

class Planet
{
public:
  Planet(double radius);
  ~Planet();

  void Initialise();

  void Update(float elapsedMS, const Camera& camera);

  // context
  // camera
  // sunDirection - unit vector indicating direction of light (from its source).
  void Draw(ContextPtr context, const Camera& camera, const glm::vec3& sunDirection);

  // Return the deepest level of detail for the current frame.
  unsigned int DeepestLoDLevel() const;

  // Return the total number of visible patches as well as the number of visible patches
  // per cube face.
  unsigned int PatchCount(unsigned int patchesPerFace[6]);

private:
  struct Impl;
  boost::scoped_ptr<Impl> impl;
};

#endif // __PLANET__
