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
  Terrain(float width, const glm::vec2& heightRange);
  ~Terrain();

  bool Initialise();

  void Update(SceneState& sceneState);

  void Draw(SceneState& sceneState);

  TerrainEffect effect;

  const float width;
  const glm::vec2 heightRange;

private:
  // Maximum quad tree depth at the highest level of detail.
  size_t maxLodLevel;

  RenderState renderState;
  VertexArray geometry;

  // The angle at which the horizon curves away, as seen by the camera.
  float horizonAngle;

  struct Node
  {
    Node(float width, size_t startLoDLevel);
    Node(const Node* const parent, size_t corner);

    void Split();
    void Merge();

    const Node* const parent;
    const float width;          // world-space unit width of the node
    const size_t lodLevel;
    const float unitWidth;      // width of a single grid square in world units
    const glm::vec3 centre;     // world-space centre of the node
    const glm::mat4 transform;  // places the node at the correct position and scale in world units
    glm::vec3 corners[4];
    boost::shared_ptr<Node> children[4];
  };

  boost::shared_ptr<Node> rootNode;
  std::vector<Node* const> visibleNodes;

  void GetVisibleNodes(const SceneState& sceneState, Node* const node);
};

#endif // __TERRAIN__
