#if ! defined(__PLANET__)
#define __PLANET__

#include <vector>
#include <glm/glm.hpp>
#include <device.h>
#include <camera.h>
#include <scenestate.h>
#include <vertexarray.h>
#include <terrain/terraineffect.h>
#include <boost/shared_ptr.hpp>

// A class to render a "planet".
class Planet
{
public:
  Planet(float radius);
  ~Planet();

  bool Initialise();

  void Update(SceneState& sceneState);

  void Draw(SceneState& sceneState);

  TerrainEffect effect;

  const float radius;

private:
  // Maximum quad tree depth at the highest level of detail.
  size_t maxLodLevel;

  RenderState renderState;
  VertexArray geometry;

  // Constant to control when patches should be split to create finer levels of detail.
  static const float maxError;

  // The angle at which the horizon curves away, as seen by the camera.
  float horizonAngle;

  // Constants defining the basic grid geometry...
  static const size_t gridSize = 9;
  static const size_t vertexCount = gridSize * gridSize;
  static const size_t indexCount = 2 * gridSize * (gridSize - 1);

  struct Node
  {
    struct Corner { enum Enum { TL, TR, BL, BR }; };
    float width;
    size_t lodLevel;
    glm::vec3 centre;
    glm::mat4 transform;  // combined scale & translate
    glm::vec3 corners[4];
    boost::shared_ptr<Node> parent;
    boost::shared_ptr<Node> children[4];
  };

  struct Face
  {
    glm::vec3 right;
    glm::vec3 forward;
    glm::vec3 up;
    glm::mat4 orientation;
    boost::shared_ptr<Node> rootNodes[4];
  };

  std::vector<Face> faces;
  std::vector<Node* const> visibleNodes;

  // Initialise one face of a cube given the _local_ frame of reference on the plane.
  void InitialiseFace(const glm::mat4& orientation, Face& face);

  boost::shared_ptr<Node> InitialiseNode(const Face& face, size_t lodLevel, float width, const glm::vec3& centre);

  void GetVisibleNodes(const glm::vec3& cameraPos, const glm::vec3& normalisedCameraPos, boost::shared_ptr<Node> node, const Face& face);

  void SplitNode(const Face& face, boost::shared_ptr<Node> parent);
};

#endif // __PLANET__
