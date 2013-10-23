/*
 The "planet" is defined as a cube. Each face of the cube is represented as a quad tree
 of nodes.
 
 There is only one unique set of geometry used to render all nodes, a flat grid on the
 XZ plane. Each face performs a model-space transform during rendering to orient the grid
 appropriately.
 
 Within the quad tree, each node defines a grid-local (ie. 2D) translation and a scaling
 factor to ensure that the standard grid is centred over the correct quadrant and covers
 the correct amount of surface area of the model.

 To create the sphere, a vertex shader first applies the world-space transform and then
 normalises the vertex before multiplying it by the planet's radius.

 Horizon culling occurs on nodes based on the angle to the horizon as seen from the
 camera position. Terrain nodes "below" the horizon are not rendered.
 */

//---------------------------------------------------------------

#include <glm/ext.hpp>
#include <climits>
#include <vector>
#include <logging.h>
#include <terrain/terrain.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//---------------------------------------------------------------

struct Vertex
{
  glm::vec2 position;
};

// Multipliers to use in order to offset a position, etc. into the correct quadrant.
// The quadrants are ordered anticlockwise, starting at the "northwest" as viewed from
// directly above the centre of the plane.
static const glm::vec3 quadrantOffsets[] =
{
  glm::vec3(-1, 0, -1),
  glm::vec3(-1, 0,  1),
  glm::vec3( 1, 0,  1),
  glm::vec3( 1, 0, -1)
};

//---------------------------------------------------------------

// Constants defining the basic grid geometry...
static const size_t gridSize = 17;
static const size_t vertexCount = gridSize * gridSize;
static const size_t indexCount = 2 * gridSize * (gridSize - 1);

//---------------------------------------------------------------

static void BuildGeometry(VertexArray& geometry);
static void BuildGrid(int size, Vertex vertices[]);
static void BuildIndices(int size, unsigned short indices[]);

//---------------------------------------------------------------

Terrain::Node::Node(float width, size_t startLoDLevel)
  : parent(NULL),
    width(width),
    lodLevel(startLoDLevel),
    unitWidth(width / (float)(gridSize - 1)),
    centre(0),
    transform(glm::scale(width, width, width))
{
  for (size_t q = 0; q < 4; ++q)
  {
    corners[q] = quadrantOffsets[q] * width * 0.5f;
  }
}

Terrain::Node::Node(const Node* const parent, size_t corner)
  : parent(parent),
    width(parent->width * 0.5f),
    lodLevel(parent->lodLevel - 1),
    unitWidth(width / (float)(gridSize - 1)),
    centre(parent->centre + (quadrantOffsets[corner] * width * 0.5f)),
    transform(glm::translate(centre) * glm::scale(width, width, width))
{
  for (size_t q = 0; q < 4; ++q)
  {
    corners[q] = centre + (quadrantOffsets[q] * width * 0.5f);
  }
}

void Terrain::Node::Split()
{
  // Don't go past maximum LOD level and don't duplicate child nodes...
  if ((lodLevel > 0) && (!children[0]))
  {
    for (size_t q = 0; q < 4; ++q)
    {
      children[q] = boost::make_shared<Node>(this, q);
    }
  }
}

void Terrain::Node::Merge()
{
  // Can't merge if already at root of quadtree...
  if (parent)
  {
    // If one child exists, then they all do...
    if (children[0])
    {
      for (size_t q = 0; q < 4; ++q)
      {
        children[q]->Merge();
        children[q].reset();
      }
    }
  }
}

//---------------------------------------------------------------

Terrain::Terrain(float width, const glm::vec2& heightRange)
  : width(width),
    heightRange(heightRange)
{
}

Terrain::~Terrain()
{
}

//---------------------------------------------------------------
bool Terrain::Initialise()
{
  // I can't remember the web site that gave the formula (!) for this but basically
  // it boils down to working out how many times the width of the largest patch
  // (in world units) must be halved to end up with a patch that represents
  // "gridSize" world units, assuming that vertices should not be closer than
  // one world unit apart (usually that equates to a distance between vertices
  // of 1 meter).
  // Anyway, for e.g. a planet with a radius of the Earth (~6300000 metres) and a grid mesh
  // width of 33 vertices, there will be 13 levels of detail (zero is the top, least detailed
  // level).
  const float log2Width = glm::log(width, 2.0f);
  const float log2Grid = glm::log(2.0f * gridSize * gridSize, 2.0f);
  maxLodLevel = size_t(glm::max(1U, (size_t)(log2Width - log2Grid)));
  
  if (!effect.Load("effects\\terrain.glsl", "Terrain")) { return false; }
  effect.MaxHeight->Set(abs(heightRange.x + heightRange.y));

  rootNode = boost::make_shared<Node>(width, maxLodLevel);

  BuildGeometry(geometry);

 // Initialise constant render state...
  renderState.effect = &effect;
  renderState.vertexArray = &geometry;
  renderState.drawState.polygonMode = GL_LINE;
  renderState.drawState.culling.enabled = true;
  renderState.drawState.culling.windingOrder = GL_CW;

  visibleNodes.resize(128);

  return true;
}

//---------------------------------------------------------------

void Terrain::Draw(SceneState& sceneState)
{
  effect.CameraPosition->Set(sceneState.camera->position);
  effect.ViewMatrix->Set(sceneState.camera->viewMatrix);
  effect.ProjectionMatrix->Set(sceneState.camera->projectionMatrix);
  effect.ViewProjectionMatrix->Set(sceneState.camera->viewProjectionMatrix);

  //// Compute angle between camera and horizon (including a simple hack to account for mountain-tops
  //// which might stick up above the horizon...
  //const float cameraHeight = sceneState.camera->position.y;
  //horizonAngle = glm::acos(width / cameraHeight) + ((cameraHeight > 1000.0) ? 20.0f : 5.0f);

  const glm::vec3 normalisedCameraPos = glm::normalize(sceneState.camera->position);

  visibleNodes.clear();
  GetVisibleNodes(sceneState, rootNode.get());

  for (size_t i = 0; i < visibleNodes.size(); ++i)
  {
    effect.WorldMatrix->Set(visibleNodes[i]->transform);

    sceneState.device->Draw(GL_TRIANGLE_STRIP, 0, indexCount, renderState);
  }
}

//---------------------------------------------------------------
void Terrain::GetVisibleNodes(const SceneState& sceneState, Node* const node)
{
  // Ignore non-existent nodes...
  if (!node)
  {
    return;
  }

  // Already at highest lod level? Just add it to the display list and return...
  if (node->lodLevel == 0)
  {
    visibleNodes.push_back(node);
    return;
  }

  // Find out whether the node's centre or one of it's corners is closest to the camera...
  //glm::vec3 transformedPoint = glm::vec3(node->transform * glm::vec4(node->centre, 1));
  float distance = glm::distance(sceneState.camera->position, node->centre);
  glm::vec3 normalisedClosestPoint = glm::normalize(node->centre);
  for (size_t i = 0; i < 4; ++i)
  {
    //transformedPoint = glm::vec3(node->transform * glm::vec4(node->corners[i], 1));
    const float distanceToCorner = glm::distance(sceneState.camera->position, node->corners[i]);
    if (distanceToCorner < distance)
    {
      distance = distanceToCorner;
      normalisedClosestPoint = glm::normalize(node->corners[i]);
    }
  }

  // Get the angle between the camera and the closest point and compare it to the angle
  // of the horizon. If greater, then this node is "below" the horizon and not visible...
  //const float dot = glm::dot(normalisedCameraPos, normalisedClosestPoint);
  //const float angle = glm::acos(dot);
  //if (angle >= horizonAngle)
  //{
  //  return;
  //}

  // Taking the centre of the node and using half it's unitWidth as the radius of a sphere,
  // compute
  // 
  //            epsilon * x
  // rho = -----------------------
  //       2 * d * tan(thetaOver2)
  //
  // where,
  // 'rho' is the screen-space error
  // 'epsilon' is the maximum allowable error metric
  // 'x' is the horizontal resolution of the display
  // 'd' is the distance to the sphere surrounding the grid square
  // 'thetaOver2' is half the camera's field of view angle
  //
  // We use the node's closest point as the centre of the sphere.
  static const float epsilon = 1.0f;
  static const float maxError = 0.1f;
  const float rho = (epsilon * sceneState.camera->displayResolutionX) / (distance * sceneState.camera->twoTanThetaover2);

  if (rho >= maxError)
  {
    // Sub-divide the node if necessary and recurse into the child nodes to check them for visibility...
    node->Split();
    for (size_t i = 0; i < 4; ++i)
    {
      GetVisibleNodes(sceneState, node->children[i].get());
    }
  }
  else
  {
    // The node has enough detail to draw...
    //node->Merge();
    visibleNodes.push_back(node);
  }
}

//---------------------------------------------------------------

// Compute a height at point p using a hybrid multifractal (Musgrave, T&M book, p502).
// @param octaves [in] number of frequencies to apply
// @param roughness [in] fractal dimension of the roughest areas
// @param lacunarity [in] "gap" between successive frequencies
// @param offset [in] fixed offset to add "above sea level"
// @return a value in the range [0, 2]
static float ComputeHeight(glm::vec3 p, float octaves, float roughness, float lacunarity, float offset)
{
  // save some computation time by caching the exponent table...
  static const size_t MaxOctaves = 20;
  static float exponents[MaxOctaves];
  static bool firstTime = true;
  if (firstTime)
  {
    firstTime = false;
    float frequency = 1;
    for (size_t i = 0; i < MaxOctaves; ++i)
    {
      exponents[i] = glm::pow(frequency, -roughness);
      frequency *= lacunarity;
    }
  }

  // clamp input:
  octaves = glm::max(octaves, (float)MaxOctaves);

  // 1st octave...
  float result = (glm::noise1(p) + offset) * exponents[0];
  float weight = result;
  p *= lacunarity;

  for (size_t i = 1; i < (size_t)octaves; ++i)
  {
    if (weight > 1.0) { weight = 1.0; }

    // get frequency above the current one:
    const float signal = (glm::noise1(p) + offset) * exponents[i];

    // add weighted frequency to result...
    result += weight * signal;

    // new weighting based on current frequency...
    weight *= signal;

    // next frequency:
    p *= lacunarity;
  }

  const float remainder = octaves - (int)octaves;
  if (remainder)
  {
    result += remainder * glm::noise1(p) * exponents[(int)octaves];
  }

  return result;
}

//---------------------------------------------------------------

static void BuildGrid(int size, Vertex vertices[])
{
  const float increment = 1.0f / (float)(size - 1);
  for (int z = 0; z < size; ++z)
  {
    for (int x = 0; x < size; ++x)
    {
      const glm::vec2 p = glm::vec2(-0.5f + (x * increment), 0.5f - (z * increment));
      vertices[x + (z * size)].position = p;
    }
  }
}

//---------------------------------------------------------------
static void BuildIndices(int size, unsigned short indices[])
{
  int i = 0;
  int z = 0;
  while (z < (size - 1))
  {
    for (int x = 0; x < size; ++x)
    {
      indices[i++] = x + (z * size);
      indices[i++] = x + ((z + 1) * size);
    }
    ++z;
    if (z < (size - 1))
    {
      for (int x = size - 1; x >= 0; --x)
      {
        indices[i++] = x + ((z + 1) * size);
        indices[i++] = x + (z * size);
      }
    }
    ++z;
  }
}

//---------------------------------------------------------------
static void BuildGeometry(VertexArray& geometry)
{
  static const VertexAttribute attributes[] =
  {
    { VertexSemantic::Position, GL_FLOAT, 2, offsetof(Vertex, position) }
  };

  VertexLayout vertexLayout;
  vertexLayout.AddAttribute(attributes[0]);

  Vertex vertices[vertexCount];
  BuildGrid(gridSize, vertices);
  boost::shared_ptr<VertexBuffer> vertexBuffer(new VertexBuffer());
  vertexBuffer->Initialise(vertexLayout, vertexCount, GL_STATIC_DRAW, vertices);

  unsigned short indices[indexCount];
  BuildIndices(gridSize, indices);
  boost::shared_ptr<IndexBuffer> indexBuffer(new IndexBuffer());
  indexBuffer->Initialise(indexCount, GL_UNSIGNED_SHORT, GL_STATIC_DRAW, indices);

  geometry.Initialise(vertexBuffer, indexBuffer);
}
