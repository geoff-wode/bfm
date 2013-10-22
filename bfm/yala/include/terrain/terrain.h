#if ! defined(__TERRAIN__)
#define __TERRAIN__

#include <vector>
#include <glm/glm.hpp>
#include <device.h>
#include <camera.h>
#include <scenestate.h>
#include <vertexarray.h>
#include <terrain/terraineffect.h>
#include <boost/shared_ptr.hpp>

// A class to render terrain.
class Terrain
{
public:
  Terrain(float width);
  ~Terrain();

  bool Initialise();

  void Update(SceneState& sceneState);

  void Draw(SceneState& sceneState);

  TerrainEffect effect;

  const float width;

private:
  // Maximum quad tree depth at the highest level of detail.
  size_t maxLodLevel;

  RenderState renderState;
  VertexArray geometry;

  // Constant to control when patches should be split to create finer levels of detail.
  static const float maxError;

  // The angle at which the horizon curves away, as seen by the camera.
  float horizonAngle;

  struct Node
  {
    struct Corner { enum Enum { TL, TR, BL, BR }; };
    Node const* parent;
    float width;
    size_t lodLevel;
    glm::vec3 centre;
    glm::mat4 transform;  // combined scale & translate
    glm::vec3 corners[4];
    boost::shared_ptr<Node> children[4];
  };

  Node rootNode;
  std::vector<Node* const> visibleNodes;

  void InitialiseRootNode();

  boost::shared_ptr<Node> InitialiseNode(const Node* const parent, const glm::vec3& offset);

  void GetVisibleNodes(const glm::vec3& cameraPos, const glm::vec3& normalisedCameraPos, Node * const node);

  void SplitNode(Node* const parent);
};

#endif // __TERRAIN__
