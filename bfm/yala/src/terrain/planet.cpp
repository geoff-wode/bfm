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
#include <terrain/planet.h>
#include <boost/shared_ptr.hpp>

//---------------------------------------------------------------

struct Vertex
{
  glm::vec3 position;
};

//---------------------------------------------------------------

// Global axes:
static const glm::vec4 Up(0, 1, 0,0);
static const glm::vec4 Down(0, -1, 0,0);
static const glm::vec4 Forward(0, 0, -1,0);
static const glm::vec4 Backward(0, 0, 1,0);
static const glm::vec4 Left(-1, 0, 0,0);
static const glm::vec4 Right(1, 0, 0,0);

const float Planet::maxError = 4;

//---------------------------------------------------------------

static void BuildGrid(int size, Vertex vertices[]);
static void BuildIndices(int size, unsigned short indices[]);

//---------------------------------------------------------------

Planet::Planet(float radius)
  : radius(radius)
{
}

Planet::~Planet()
{
}

//---------------------------------------------------------------
bool Planet::Initialise()
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
  const float log2r = glm::log(2 * radius, 2.0f);
  const float log2grid = glm::log(2 * (float)gridSize * (float)gridSize, 2.0f);
  maxLodLevel = glm::max(1U, (size_t)(log2r - log2grid));

  if (!effect.Load("effects\\terrain.glsl", "Terrain")) { return false; }

  faces.resize(1);
  InitialiseFace(glm::rotate(90.0f, 1.0f, 0.0f, 0.0f), faces[0]); // front
  //InitialiseFace(glm::rotate(90.0f, 1.0f, 1.0f, 0.0f) * glm::rotate(90.0f, 0.0f, 1.0f, 0.0f), faces[1]);

  //InitialiseFace(Forward, Up, faces[1]);      // right
  //InitialiseFace(Left, Up, faces[2]);         // back
  //InitialiseFace(Backward, Up, faces[3]);     // left
  //InitialiseFace(Right, Forward, faces[4]);   // top
  //InitialiseFace(Right, Backward, faces[5]);  // bottom

  static const VertexAttribute attributes[] =
  {
    { VertexSemantic::PositionLow, GL_FLOAT, 3, offsetof(Vertex, position) }
  };

  VertexLayout vertexLayout;
  vertexLayout.AddAttribute(attributes[0]);

  // A single vertex buffer for all 6 faces of the cube...
  Vertex vertices[vertexCount];
  BuildGrid(gridSize, vertices);
  boost::shared_ptr<VertexBuffer> vertexBuffer(new VertexBuffer());
  vertexBuffer->Initialise(vertexLayout, vertexCount, GL_STATIC_DRAW, vertices);

  // Index buffer...
  unsigned short indices[indexCount];
  BuildIndices(gridSize, indices);
  boost::shared_ptr<IndexBuffer> indexBuffer(new IndexBuffer());
  indexBuffer->Initialise(indexCount, GL_UNSIGNED_SHORT, GL_STATIC_DRAW, indices);

  // Create final vertex array:
  geometry.Initialise(vertexBuffer, indexBuffer);

  // Initialise constant render state...
  renderState.effect = &effect;
  renderState.vertexArray = &geometry;
  renderState.drawState.polygonMode = GL_LINE;
  renderState.drawState.culling.enabled = false;
  renderState.drawState.culling.windingOrder = GL_CW;

  visibleNodes.resize(128);

  return true;
}

//---------------------------------------------------------------
void Planet::InitialiseFace(const glm::mat4& orientation, Face& face)
{
  face.orientation = orientation;

  for (size_t i = 0; i < 4; ++i)
  {
    const float halfWidth = radius * 0.5f;
    glm::vec3 centre;
    switch (i)
    {
    case Node::Corner::TL:
      centre = glm::vec3(-halfWidth, 0.0, -halfWidth);
      break;
    case Node::Corner::TR:
      centre = glm::vec3(halfWidth, 0.0, -halfWidth);
      break;
    case Node::Corner::BL:
      centre = glm::vec3(-halfWidth, 0.0, halfWidth);
      break;
    case Node::Corner::BR:
      centre = glm::vec3(halfWidth, 0.0, halfWidth);
      break;
    }
    face.rootNodes[i] = InitialiseNode(face, 1, halfWidth, centre);
    face.rootNodes[i]->parent = NULL;
  }
}

//---------------------------------------------------------------
boost::shared_ptr<Planet::Node> Planet::InitialiseNode(const Face& face, size_t lodLevel, float width, const glm::vec3& centre)
{
  boost::shared_ptr<Node> node(new Node());
  
  node->lodLevel = lodLevel;
  node->width = width;
  node->centre = centre;

  const glm::mat4 translate = glm::translate(centre);
  const glm::mat4 scale = glm::scale((face.forward + face.right) * width);
  node->transform = translate * scale;

  const float halfWidth = width * 0.5f;
  node->corners[Node::Corner::TL] = node->centre + glm::vec3(-halfWidth, 0.0, -halfWidth);
  node->corners[Node::Corner::TR] = node->centre + glm::vec3( halfWidth, 0.0, -halfWidth);
  node->corners[Node::Corner::BL] = node->centre + glm::vec3(-halfWidth, 0.0,  halfWidth);
  node->corners[Node::Corner::BR] = node->centre + glm::vec3( halfWidth, 0.0,  halfWidth);

  return node;
}

//---------------------------------------------------------------

void Planet::Draw(SceneState& sceneState)
{
  const glm::mat4 vp = sceneState.camera->projectionMatrix * sceneState.camera->viewMatrix;

  // Compute angle between camera and horizon (including a simple hack to account for mountain-tops
  // which might stick up above the horizon...
  const float cameraHeight = glm::length(sceneState.camera->position);
  horizonAngle = glm::acos(radius / cameraHeight) + ((cameraHeight > 1000.0) ? 20.0f : 5.0f);

  const glm::vec3 normalisedCameraPos = glm::normalize(sceneState.camera->position);

  for (size_t f = 0; f < faces.size(); ++f)
  {
    visibleNodes.clear();
    for (size_t n = 0; n < 4; ++n)
    {
      GetVisibleNodes(sceneState.camera->position, normalisedCameraPos, faces[f].rootNodes[n], faces[f]);
    }

    for (size_t i = 0; i < visibleNodes.size(); ++i)
    {
      const glm::mat4 world = faces[f].orientation * visibleNodes[i]->transform;
      const glm::mat4 wvp = vp * world;
      effect.WorldViewProjectionMatrix->Set(wvp);
      effect.Centre->Set(visibleNodes[i]->centre);
      effect.Width->Set(visibleNodes[i]->width);

      sceneState.device->Draw(GL_TRIANGLE_STRIP, 0, indexCount, renderState);
    }
  }
}

//---------------------------------------------------------------
void Planet::GetVisibleNodes(const glm::vec3& cameraPos, const glm::vec3& normalisedCameraPos, boost::shared_ptr<Node> node, const Face& face)
{
  // Ignore non-existent nodes...
  if (!node)
  {
    return;
  }

  // Already at highest lod level? Just add it to the display list and return...
  if (node->lodLevel == maxLodLevel)
  {
    visibleNodes.push_back(node.get());
    return;
  }

  // Find the distance to and surface angle of the nearest corner of the node...
  float distance = FLT_MAX;
  size_t nearestCorner = 0;
  for (size_t i = 0; i < 4; ++i)
  {
    const float distanceToCorner = glm::distance(cameraPos, node->corners[i]);
    if (distanceToCorner < distance)
    {
      nearestCorner = i;
      distance = distanceToCorner;
    }
  }

  // Get the angle between the camera and the closest point and compare it to the angle
  // of the horizon. If greater, then this node is "below" the horizon and not visible...
  //const glm::vec3 norm = glm::normalize(node->corners[nearestCorner]);
  //const float dot = glm::dot(normalisedCameraPos, norm);
  //const float angle = glm::acos(dot);
  //if (angle >= horizonAngle)
  //{
  //  return;
  //}

  //// Now ensure that the level of detail is high enough (within the pre-computed maximum).
  //const float error = distance / node->width;
  //if ((error < maxError) && (node->lodLevel < maxLodLevel))
  //{
  //  // Sub-divide the node if necessary and recurse into the child nodes to check them for visibility...
  //  if (!node->children[0])
  //  {
  //    SplitNode(face, node);
  //  }
  //  for (size_t i = 0; i < 4; ++i)
  //  {
  //    GetVisibleNodes(cameraPos, normalisedCameraPos, node->children[i], face);
  //  }
  //}
  //else
  {
    // The node has enough detail to draw...
    visibleNodes.push_back(node.get());
  }
}

//---------------------------------------------------------------
void Planet::SplitNode(const Face& face, boost::shared_ptr<Node> parent)
{
  const float quarterWidth = parent->width * 0.25f;
  for (size_t i = 0; i < 4; ++i)
  {
    const float quarterWidth = radius * 0.25f;
    glm::vec3 centre = face.up * 0.5f;
    switch (i)
    {
    case Node::Corner::TL:
      centre += (face.right * quarterWidth) + (face.forward * quarterWidth);
      break;
    case Node::Corner::TR:
      centre += (face.right * quarterWidth) + (face.forward * quarterWidth);
      break;
    case Node::Corner::BL:
      centre += (face.right * quarterWidth) - (face.forward * quarterWidth);
      break;
    case Node::Corner::BR:
      centre += (face.right * quarterWidth) - (face.forward * quarterWidth);
      break;
    }
    parent->children[i] = InitialiseNode(face, parent->lodLevel + 1, radius * 0.5f, centre);
    parent->children[i]->parent = parent;
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
  const glm::vec3 base = glm::vec3(-0.5f, 0.0f, 0.5f);
  for (int z = 0; z < size; ++z)
  {
    for (int x = 0; x < size; ++x)
    {
      const glm::vec3 p = base + glm::vec3(x * increment, 0.0, z * increment);
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
