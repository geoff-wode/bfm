#include <climits>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <core/device.h>
#include <core/drawstate.h>
#include <game/planet/planet.h>
#include <game/planet/planeteffect.h>

//---------------------------------------------------------------------------

static const double maxError = 4.0;
static const unsigned int gridSize = 17;
static const unsigned int vertexCount = gridSize * gridSize;
static const unsigned int indexCount = (gridSize - 1) * (gridSize - 1) * 6;
static const unsigned int primitiveCount = indexCount / 3;

//---------------------------------------------------------------------------

// A quadtree patch.
typedef boost::shared_ptr<struct Patch> PatchPtr;

struct Patch
{
  struct Corner
  {
    enum Enum { TL, TR, BL, BR };
  };

  unsigned int level;
  double width;
  glm::dvec3 centre;
  glm::dvec3 corners[4];
  PatchPtr parent;
  PatchPtr children[4];
};

//---------------------------------------------------------------------------

// One face of a cube.
struct Face
{
  Face() : rootNode(new Patch()) { }

  glm::dvec3 right;
  glm::dvec3 forward;
  glm::dvec3 up;

  PatchPtr rootNode;

  // RAW pointers NOT shared ones, since clearing/copying shared pointers carries
  // overhead of at least managing the reference counter, which is redundant because
  // memory pointed to in this list is never deallocated from within the list.
  std::vector<Patch*> visiblePatches;
};

typedef boost::shared_ptr<Face> FacePtr;

//---------------------------------------------------------------------------

struct Planet::Impl
{
  Impl(double radius)
    : radius(radius),
      maxLevel(unsigned int(glm::log2(radius * 2 * 1000) - glm::log2(gridSize * gridSize)))
  { }

  const double radius;
  const unsigned int maxLevel;

  double horizonAngle;
  unsigned int deepestLoDLevel;

  FacePtr faces[6];

  PlanetEffect effect;
  DrawState drawState;

  VertexLayout vertexLayout;

  void GetVisiblePatches(const Camera& camera, const unsigned int maxLevel);
  void GetVisiblePatches(const Camera& camera, const unsigned int maxLevel, FacePtr face, PatchPtr patch);
  void SplitNode(FacePtr face, PatchPtr parent, Patch::Corner::Enum corner);
};

//---------------------------------------------------------------------------

struct Vertex
{
  glm::vec3 position;
  glm::vec2 textureCoord;
};

//---------------------------------------------------------------------------

static void CreateFace(double radius, const glm::dvec3& right, const glm::dvec3& forward, FacePtr face);
static void InitPatch(FacePtr face, unsigned int level, double width, const glm::dvec3& centre, PatchPtr patch);
static void CreateVertices(Vertex* const vertices, const glm::dvec3& right, const glm::dvec3& forward);
static void CreateIndices(std::vector<unsigned short>& indices);

//---------------------------------------------------------------------------

Planet::Planet(double radius)
  : impl(new Impl(radius))
{
  std::memset(impl.get(), 0, sizeof(Impl));
}

//---------------------------------------------------------------------------

Planet::~Planet()
{
}

//---------------------------------------------------------------------------

unsigned int Planet::DeepestLoDLevel() const { return impl->deepestLoDLevel; }

//---------------------------------------------------------------------------

unsigned int Planet::PatchCount(unsigned int patchesPerFace[6])
{
  unsigned int totalPatches = 0;
  for (int i = 0; i < 6; ++i)
  {
    patchesPerFace[i] = impl->faces[i]->visiblePatches.size();
    totalPatches += patchesPerFace[i];
  }
  return totalPatches;
}

//---------------------------------------------------------------------------

void Planet::Initialise()
{
  // Create the cube that represents the spherical planet...
  {
    static const glm::dvec3 up(0,1,0);
    static const glm::dvec3 right(1,0,0);
    static const glm::dvec3 forward(0,0,1);
    static const glm::dvec3 down(0,-1,0);
    static const glm::dvec3 left(-1,0,0);
    static const glm::dvec3 backward(0,0,-1);
    CreateFace(impl->radius, right, up, impl->faces[0]);        // front
    CreateFace(impl->radius, forward, up, impl->faces[1]);      // right
    CreateFace(impl->radius, left, up, impl->faces[2]);         // back
    CreateFace(impl->radius, backward, up, impl->faces[3]);     // left
    CreateFace(impl->radius, right, forward, impl->faces[4]);   // top
    CreateFace(impl->radius, right, backward, impl->faces[5]);  // bottom
  }

  // Create the geometry...
  {
    static const VertexAttribute vertexAttributes[] =
    {
      { VertexSemantic::Position, GL_FLOAT, 3, offsetof(Vertex, position) },
      { VertexSemantic::Texture0, GL_FLOAT, 2, offsetof(Vertex, textureCoord) }
    };
    static const unsigned int attributeCount = sizeof(vertexAttributes) / sizeof(vertexAttributes[0]);

    for (int i = 0; i < attributeCount; ++i)
    {
      impl->vertexLayout.AddAttribute(vertexAttributes[i]);
    }

    std::vector<Vertex> vertices(vertexCount * 6);
    for (int i = 0; i < 6; ++i)
    {
      CreateVertices(&vertices[vertexCount * 6], impl->faces[i]->right, impl->faces[i]->forward);
    }

    std::vector<unsigned short> indices(indexCount);
    CreateIndices(indices);

    VertexBufferPtr vertexBuffer = Device::NewVertexBuffer(impl->vertexLayout, vertexCount, GL_STATIC_DRAW);
    IndexBufferPtr indexBuffer = Device::NewIndexBuffer(indexCount, GL_UNSIGNED_SHORT, GL_STATIC_DRAW);
    impl->drawState.vertexArray = Device::NewVertexArray(vertexBuffer, indexBuffer);
  }

  // Initialise the effect and its constant uniform parameters...
  {
    impl->effect.Load("assets/effects/planet.glsl");
    impl->effect.Radius->Set(impl->radius);
    impl->effect.WorldMatrix->Set(glm::mat4(1));
    impl->drawState.effect = &impl->effect;
  }

  impl->drawState.renderState.mode = GL_LINE;
}

//---------------------------------------------------------------------------

void Planet::Update(float elapsedMS, const Camera& camera)
{
  // Find the angle between the camera position and the horizon...
  const double height = glm::length(camera.position);
  impl->horizonAngle = glm::acos(impl->radius / height);
  // then add a small "fudge factor" for mountain tops that peek above the spherical horizon...
  impl->horizonAngle += (height > 1000) ? 20 : 5;

  // Get the set of currently visible terrain patches...
  impl->GetVisiblePatches(camera, 0);
}

//---------------------------------------------------------------------------

void Planet::Draw(ContextPtr context, const Camera& camera, const glm::vec3& sunDirection)
{
  impl->effect.SunDirection->Set(sunDirection);
  impl->effect.WorldMatrix->Set(glm::mat4(1));
  impl->effect.ViewMatrix->Set(camera.viewMatrix);
  impl->effect.ProjectionMatrix->Set(camera.projectionMatrix);
  impl->effect.WorldViewProjectionMatrix->Set(camera.projectionMatrix * camera.viewMatrix);

  for (int face = 0; face < 6; ++face)
  {
    BOOST_FOREACH(auto patch, impl->faces[face]->visiblePatches)
    {
      impl->effect.Centre->Set(patch->centre);
      impl->effect.Width->Set(patch->width);
      impl->effect.Apply();
      
      context->Draw(GL_TRIANGLES, vertexCount, vertexCount * face, impl->drawState);
    }
  }
}

//---------------------------------------------------------------------------

static void CreateFace(double radius, const glm::dvec3& right, const glm::dvec3& forward, FacePtr face)
{
  face->right = right;
  face->forward = forward;
  face->up = glm::cross(right, forward);
  InitPatch(face, 0, radius * 2, radius * face->up, face->rootNode);
}

//---------------------------------------------------------------------------

static void InitPatch(FacePtr face, unsigned int level, double width, const glm::dvec3& centre, PatchPtr patch)
{
  patch->level = level;
  patch->width = width;
  patch->centre = centre;

  const glm::dvec3 right = face->right * width * 0.5;
  const glm::dvec3 forward = face->forward * width * 0.5;
  patch->corners[Patch::Corner::TL] = centre - right + forward;
  patch->corners[Patch::Corner::TR] = centre + right + forward;
  patch->corners[Patch::Corner::BL] = centre - right - forward;
  patch->corners[Patch::Corner::BR] = centre + right - forward;
}

//---------------------------------------------------------------------------

static void CreateVertices(Vertex* const vertices, const glm::dvec3& right, const glm::dvec3& forward)
{
  static const double inc = 1.0 / double(gridSize - 1);
  const glm::dvec3 start = (right + forward) * -0.5;
  for (unsigned int z = 0; z < gridSize; ++z)
  {
    for (unsigned int x = 0; x < gridSize; ++x)
    {
      const glm::dvec3 p = start + (right * inc * double(x)) + (forward * inc * double(z));
      const glm::vec2 t = glm::vec2(inc * x, inc * z);
      vertices[x + (z * gridSize)].position = p;
      vertices[x + (z * gridSize)].textureCoord = t;
    }
  }
}

//---------------------------------------------------------------------------

static void CreateIndices(std::vector<unsigned short>& indices)
{
  unsigned int counter = 0;
  for (unsigned int x = 0; x < gridSize - 1; ++x)
  {
    for (unsigned int z = 0; z < gridSize - 1; ++z)
    {
      short lowerLeft = (short)(z + (x * gridSize));
      short lowerRight = (short)((z + 1) + (x * gridSize));
      short topLeft = (short)(z + ((x + 1) * gridSize));
      short topRight = (short)((z + 1) + ((x + 1) * gridSize));

      indices[counter++] = topLeft;
      indices[counter++] = lowerRight;
      indices[counter++] = lowerLeft;

      indices[counter++] = topLeft;
      indices[counter++] = topRight;
      indices[counter++] = lowerRight;
    }
  }
}

//---------------------------------------------------------------------------

void Planet::Impl::GetVisiblePatches(const Camera& camera, const unsigned int maxLevel)
{
  deepestLoDLevel = 0;

  for (int i = 0; i < 6; ++i)
  {
    faces[i]->visiblePatches.clear();
    GetVisiblePatches(camera, maxLevel, faces[i], faces[i]->rootNode);
  }
}

//---------------------------------------------------------------------------

void Planet::Impl::GetVisiblePatches(const Camera& camera, const unsigned int maxLevel, FacePtr face, PatchPtr patch)
{
  if (!patch) { return; }

  if (patch->level > deepestLoDLevel)
  {
    deepestLoDLevel = patch->level;
  }

  if (patch->level == maxLevel)
  {
    face->visiblePatches.push_back(patch.get());
    return;
  }

  double shortestDistance = DBL_MAX;
  double angle = 0.0;

  // Find distance to the corner closest to the camera and the angle between them...
  for (int i = 0; i < 4; ++i)
  {
    const double cornerDistance = glm::distance(camera.position, patch->corners[i]);
    if (cornerDistance < shortestDistance)
    {
      shortestDistance = cornerDistance;
      angle = glm::acos(glm::normalizeDot(camera.position, patch->corners[i]));
    }
  }

  // Only proceed if the nearest corner is "above" the horizon...
  if (angle > horizonAngle) { return; }

  // If the level of detail on the current patch is not high enough, split it and recurse into each
  // child patch. If the LoD is high enough, the patch is added to the visible set.

  const double epsilon = shortestDistance / patch->width;
  if (epsilon < maxError)
  {
    // Split the patch and recurse...
    for (int i = 0; i < 4; ++i)
    {
      if (!patch->children[i])
      {
        SplitNode(face, patch, Patch::Corner::Enum(i));
      }
      GetVisiblePatches(camera, maxLevel, face, patch->children[i]);
    }
  }
  else
  {
    // The current patch is already detailed enough...
    face->visiblePatches.push_back(patch.get());
  }
}

//---------------------------------------------------------------------------

void Planet::Impl::SplitNode(FacePtr face, PatchPtr parent, Patch::Corner::Enum corner)
{
  if (parent->level < maxLevel)
  {
    const glm::dvec3 right = face->right * parent->width * 0.25;
    const glm::dvec3 forward = face->forward * parent->width * 0.25;

    glm::dvec3 centre;
    switch (corner)
    {
    case Patch::Corner::TL: centre = parent->centre - right + forward; break;
    case Patch::Corner::TR: centre = parent->centre + right + forward; break;
    case Patch::Corner::BL: centre = parent->centre - right - forward; break;
    case Patch::Corner::BR: centre = parent->centre + right - forward; break;
    default: return;
    }

    PatchPtr newNode(new Patch());
    InitPatch(face, parent->level + 1, parent->width * 0.5, centre, newNode);
    newNode->parent = parent;
    parent->children[int(corner)] = newNode;
  }
}
