#include <glm/ext.hpp>
#include <terrain.h>
#include <gl_loader/gl_loader.h>
#include <boost/make_shared.hpp>

//-------------------------------------------------------------------------

static const size_t vertexCount = gridSize * gridSize;
static const size_t indexCount = 2 * gridSize * (gridSize - 1);
static const double vertexOffset = (float)gridSize * 0.5f;


// The maximum screen error determines how/when a tile is subdivided to increase
// the visible resolution. If the tile's screen error is >= this maximum, it is
// subdivided.
static const double maxScreenError = 0.001;

//-------------------------------------------------------------------------

struct Vertex
{
  glm::vec3 position;
};

//-------------------------------------------------------------------------

// Define the matrices that, when applied to a point, transform it to its
// world position on a terrain plane.
static const glm::dmat4 quadrentTranslation[] =
{
  glm::dmat4(glm::translate( 0.25, 0.0, -0.25)),  // ne
  glm::dmat4(glm::translate(-0.25, 0.0, -0.25)),  // nw
  glm::dmat4(glm::translate( 0.25, 0.0,  0.25)),  // se
  glm::dmat4(glm::translate(-0.25, 0.0,  0.25))   // sw
};
//-------------------------------------------------------------------------

static boost::shared_ptr<Mesh> InitialiseMesh();
static void InitialiseTileHeightData(int depth, const glm::dmat4& worldMatrix, double planetRadius, double min, double max, float heights[]);

// Hybrid multifractal (T&M p502).
static double FractalHeight(
  glm::dvec3 p,               // the point to compute the fractal at
  double octaves = 4,         // number of iterations - note it can include a fractional part
  double dimension = 0.25,    // fractal "dimension" - determines maximum roughness
  double lacunarity = 2.012,  // gap between successive frequencies
  double offset = 0.25);       // sea level offset

//-------------------------------------------------------------------------
Terrain::Terrain(double width, const glm::dvec2& mountainMinMax)
  : radius(width * 0.5),
    mountainMinMax(mountainMinMax)
{
}

Terrain::~Terrain()
{
}

//-------------------------------------------------------------------------
void Terrain::Initialise()
{
  baseMesh = InitialiseMesh();
  renderState.vertexArray = baseMesh->vertexArray.get();

  const glm::dvec3 bottomLeft(-radius, 0.0, radius);
  const glm::dvec3 topRight(radius, 0.0, -radius);
  const double distance = glm::distance(topRight, bottomLeft);

  for (size_t i = 0; i < 4; ++i)
  {
    children[i] = boost::make_shared<TerrainTile>();

    children[i]->depth = 0;
    children[i]->width = distance;

    // Each tile has a world-space scale and translate applied to it to place it into
    // the final scene...
    children[i]->worldMatrix = quadrentTranslation[i] * glm::scale(glm::dvec3(radius * 0.5));

    // The bounding sphere has a diameter that goes diagonally across the tile from corner to corner.
    // Without this, the corners of the tile would not be included in visibility tests.
    children[i]->boundingSphere.centre = glm::dvec3(quadrentTranslation[i] * glm::dvec4(radius));
    children[i]->boundingSphere.radius = distance * 0.5;

    InitialiseTileHeightData(
      children[i]->depth,
      children[i]->worldMatrix,
      radius,
      mountainMinMax.x, mountainMinMax.y,
      children[i]->heightData);

    children[i]->heightMap = boost::make_shared<Texture2D>();
    children[i]->heightMap->LoadHeightMap(children[i]->heightData, gridSize, gridSize);
  }
}

//-------------------------------------------------------------------------
void Terrain::Update(const glm::dvec3& cameraPos)
{
}

//-------------------------------------------------------------------------
void Terrain::Draw(Device& device, Effect& effect, const Camera& camera)
{
  renderState.effect = &effect;

  for (size_t i = 0; i < 4; ++i)
  {
    const glm::mat4 worldMatrix = glm::mat4(children[i]->worldMatrix);
    const glm::mat4 worldViewMatrix = camera.viewMatrix * worldMatrix;
    const glm::mat4 wvpMatrix = camera.viewProjectionMatrix * worldMatrix;

    renderState.effect->WorldMatrix.Set(worldMatrix);
    renderState.effect->WorldViewMatrix.Set(worldViewMatrix);
    renderState.effect->WorldViewProjectionMatrix.Set(wvpMatrix);

    renderState.textureUnits[0] = children[i]->heightMap;
    device.Draw(GL_TRIANGLE_STRIP, 0, baseMesh->vertexArray->GetVertexBuffer()->GetVertexCount(), renderState);
  }
}

//-------------------------------------------------------------------------
static boost::shared_ptr<Mesh> InitialiseMesh()
{
  VertexLayout layout;
  layout.AddAttribute(VertexAttribute(VertexSemantic::Position, GL_FLOAT_VEC3, offsetof(Vertex, position)));

  boost::shared_ptr<VertexBuffer> vb(new VertexBuffer(layout, vertexCount, GL_STATIC_DRAW));
  boost::shared_ptr<IndexBuffer> ib(new IndexBuffer(indexCount, GL_UNSIGNED_SHORT, GL_STATIC_DRAW));

  {
    std::vector<Vertex> vertices(vertexCount);
    for (size_t x = 0; x < gridSize; ++x)
    {
      for (size_t z = 0; z < gridSize; ++z)
      {
        const glm::vec3 p = glm::vec3((float)x - vertexOffset, 0.0f, (float)z + vertexOffset);
        vertices[x + (z*gridSize)].position = p;
      }
    }
    vb->Enable();
    vb->SetData(vertices.data(), vertexCount);
    vb->Disable();
  }

  {
    std::vector<GLushort> indices(indexCount);
    size_t i = 0;
    int z = 0;
    while (z < (int)gridSize - 1)
    {
      for (int x = 0; x < (int)gridSize; ++x)
      {
        indices[i++] = x + (z * (int)gridSize);
        indices[i++] = x + ((z+1) * (int)gridSize);
      }
      ++z;
      if (z < (int)gridSize - 1)
      {
        for (int x = (int)gridSize - 1; x >= 0; --x)
        {
          indices[i++] = x + ((z+1) * (int)gridSize);
          indices[i++] = x + (z * (int)gridSize);
        }
      }
      ++z;
    }
    ib->Enable();
    ib->SetData(indices.data(), indexCount);
    ib->Disable();
  }

  boost::shared_ptr<VertexArray> va(new VertexArray(vb, ib));

  boost::shared_ptr<Mesh> mesh(new Mesh());
  mesh->vertexArray = va;

  return mesh;
}

//-------------------------------------------------------------------------
static void InitialiseTileHeightData(int depth, const glm::dmat4& worldMatrix, double planetRadius, double min, double max, float heights[])
{
  for (size_t x = 0; x < gridSize; ++x)
  {
    for (size_t z = 0; z < gridSize; ++z)
    {
      // To generate a height value for a terrain point, compute a point on a unit cube, then
      // normalize to create a unit sphere and finally scale by the radius of the planet to
      // find the point on the surface...
      const glm::dvec4 pointOnPlane = glm::dvec4((double)x - vertexOffset, 1.0, (double)z + vertexOffset, 1);
      const glm::dvec4 pointOnCube = worldMatrix * pointOnPlane;

      const glm::dvec3 pointOnSphere = planetRadius * glm::dvec3(glm::normalize(pointOnCube));

      // Now run the fractal...
      const double fractal = FractalHeight(pointOnSphere, 4 + depth);
      // ...and convert the [-1,+1] range into something useful...
      const double height = 0; //glm::mix(min, max, (1 + fractal) * 0.5);
      heights[x + (z*gridSize)] = (float)height;
    }
  }
}

//-------------------------------------------------------------------------
static double FractalHeight(glm::dvec3 p, double octaves, double dimension, double lacunarity, double offset)
{
  static const int MaxOctaves = 16;

  // Save a little time by storing the exponential values in a lookup table...
  static bool first = true;
  static double exponents[MaxOctaves];
  if (first)
  {
    double frequency = 1.0;
    for (size_t i = 0; i < (size_t)MaxOctaves; ++i)
    {
      exponents[i] = pow(frequency, -dimension);
      frequency *= lacunarity;
    }
    first = false;
  }

  octaves = glm::clamp(octaves, 0.0, (double)MaxOctaves);

  // first octave...
  double result = (glm::perlin(p) + offset) * exponents[0];
  double weight = result;

  // increase frequency...
  p *= lacunarity;

  // build fractal (starting at 2nd octave)...
  for (int i = 1; i < (int)octaves; ++i)
  {
    // clamp to prevent the algorithm blowing up...
    if (weight > 1.0) { weight = 1.0; }

    // next frequency...
    const double signal = (glm::perlin(p) + offset) * exponents[i];

    // add it in, weighted by previous frequency...
    result += weight * signal;

    // update weighting value for the next loop...
    weight *= signal;

    // increase frequency...
    p *= lacunarity;
  }

  // handle remainder of 'octaves'...
  const double remainder = octaves - (int)octaves;
  if (remainder > 0.0)
  {
    result += (remainder * glm::perlin(p) * exponents[(int)octaves + 1]);
  }

  return result;
}
