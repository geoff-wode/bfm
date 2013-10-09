#if ! defined(__TERRAIN__)
#define __TERRAIN__

#include <glm/glm.hpp>
#include <mesh.h>
#include <texture2d.h>
#include <device.h>
#include <effect.h>
#include <camera.h>
#include <boost/shared_ptr.hpp>

struct BoundingSphere
{
  double radius;
  glm::dvec3 centre;
};

static const size_t gridSize = 33;

struct TerrainTile
{
  size_t depth;   // quadtree depth, used to compute fractals and limit recursion
  double width;   // diagonal distance in world units from one corner to the other
  glm::dmat4 worldMatrix;
  BoundingSphere boundingSphere;
  boost::shared_ptr<Texture2D>  heightMap;
  boost::shared_ptr<struct TerrainTile> children[4];
  float heightData[gridSize * gridSize];
};
  
class Terrain
{
public:
  Terrain(double width, const glm::dvec2& mountainMinMax);
  ~Terrain();

  void Initialise();

  void Update(const glm::dvec3& cameraPos);
  void Draw(Device& device, Effect& effect, const Camera& camera);

private:
  // The distance from the centre of the terrain to the axis-aligned outer edge...
  double radius;
  glm::dvec2 mountainMinMax;

  RenderState renderState;

  // Root level of the tile quadtree...
  boost::shared_ptr<TerrainTile> children[4];

  // The underlying geometry sent to the GPU is always the same and is kept here...
  boost::shared_ptr<Mesh> baseMesh;
};

#endif // __TERRAIN__
