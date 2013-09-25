#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <gl_loader/gl_loader.h>
#include <vector>
#include <effect.h>
#include <texture2d.h>
#include <terrain.h>
#include <timer.h>
#include <vao.h>
#include <logging.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

//-----------------------------------------------------------
struct Vertex
{
  glm::vec3 position;
};

struct TerrainTile
{
  float size;           // distance from one edge to the other in world units
  glm::vec3 centre;     // location of tile centre in world units
  glm::mat4 transform;
  boost::shared_ptr<Texture2D> heightMap;

  boost::shared_ptr<TerrainTile> children[4];
};

//-----------------------------------------------------------

static double Fractal(
  const glm::vec3& point,
  double octaves = 4,
  double lacunarity = 2,
  double roughness = 0.1,
  double offset = 0.8);

static boost::shared_ptr<Texture2D> CreateHeightMapTexture(boost::shared_ptr<TerrainTile> tile);

static void InitialiseFace(TerrainTile& face, const glm::mat4& transform, float radius);

//-----------------------------------------------------------

static boost::shared_ptr<VAO> vao;
static std::vector<Vertex> tileVertices;
static std::vector<GLshort> tileIndices;
static TerrainTile cubeFaces[6];

//-----------------------------------------------------------

void TerrainInitialise(float radius)
{
  Timer timer;
  timer.Start();

  vao = boost::make_shared<VAO>();

  Assimp::Importer importer;
  const char* const modelName = "assets\\plane.3ds";
  const aiScene* scene = importer.ReadFile(modelName, 0);
  if (!scene)
  {
    LOG("could not load %s: %s\n", modelName, importer.GetErrorString());
    return;
  }

  const aiMesh* mesh = scene->mMeshes[0];
  for (size_t i = 0; i < mesh->mNumVertices; ++i)
  {
    Vertex v;
    const aiVector3D* pos = &(mesh->mVertices[i]);
    v.position = glm::vec3(pos->x, 1.0f, pos->z);
    tileVertices.push_back(v);
  }

  for (size_t i = 0; i < mesh->mNumFaces; ++i)
  {
    const aiFace& face = mesh->mFaces[i];
    tileIndices.push_back(face.mIndices[0]);
    tileIndices.push_back(face.mIndices[1]);
    tileIndices.push_back(face.mIndices[2]);
  }
  ASSERT(tileIndices.size() <= USHRT_MAX);

  vao->indexCount = tileIndices.size();
  vao->vertexBuffer = boost::make_shared<VertexBuffer>(sizeof(Vertex), tileVertices.size(), GL_STATIC_DRAW, tileVertices.data());
  vao->indexBuffer = boost::make_shared<IndexBuffer>(GL_UNSIGNED_SHORT, sizeof(GLushort), tileIndices.size(), GL_STATIC_DRAW, tileIndices.data());
  vao->Bind();
  vao->vertexBuffer->Bind();
  vao->indexBuffer->Bind();

  glEnableVertexAttribArray(SHADER_SEMANTIC_POSITION);
  glVertexAttribPointer(SHADER_SEMANTIC_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

  vao->Unbind();
  VertexBuffer::Unbind();
  IndexBuffer::Unbind();

  // Initialise the top-level tile on each of the cube's faces...

  static const glm::mat4 CubeFaceTransforms[] =
  {
    glm::mat4(),                                                                  // top - no transform needed
    glm::rotate(90.0f, glm::vec3(1,0,0)),                                         // front - rotated 90 degrees anti-clockwise around X axis
    glm::rotate(180.0f, glm::vec3(1,0,0)),                                        // bottom - rotated 180 degrees around X axis
    glm::rotate(270.0f, glm::vec3(1,0,0)),                                        // bottom - rotated 270 degrees around X axis
    glm::rotate(90.0f, glm::vec3(0,0,-1)) * glm::rotate(90.0f, glm::vec3(0,1,0)), // left - rotate 90 in Y then 90 in Z
    glm::rotate(270.0f, glm::vec3(0,0,-1)) * glm::rotate(270.0f, glm::vec3(0,1,0))// right - rotate 270 in Y then 270 in Z
  };

  for (size_t i = 0; i < 6; ++i)
  {
    InitialiseFace(cubeFaces[i], CubeFaceTransforms[i], radius);
  }

  timer.Stop();
  LOG("terrain initialised in %fs\n", (float)timer.elapsedMS/1000.0f);
}

//-----------------------------------------------------------
void TerrainDraw(const glm::mat4& cameraViewProjectionMatrix, Effect& effect)
{
  vao->Bind();

  for (size_t i = 0; i < 6; ++i)
  {
    if (!cubeFaces[i].children[0])
    {
      const glm::mat4 wvpMatrix = cameraViewProjectionMatrix * cubeFaces[i].transform;
      glProgramUniformMatrix4fv(effect.handle, effect.params["WorldViewProjectionMatrix"], 1, GL_FALSE, glm::value_ptr(wvpMatrix));
      cubeFaces[i].heightMap->Bind(0);
      glDrawElements(GL_TRIANGLES, vao->indexCount, GL_UNSIGNED_SHORT, 0);
    }
    else
    {
      // TODO: recursive draw
    }
  }

  vao->Unbind();
}

//-----------------------------------------------------------
// Return a fractal value for the location of 'point'.
static double Fractal(
  const glm::vec3& point,
  double octaves,
  double lacunarity,
  double roughness,
  double offset)
{
  static const int MaxOctaves = 16;
  static double exponentArray[MaxOctaves];
  static bool first = true;

  if (first)
  {
    // Pre-compute the exponents for speed...
    first = false;
    double frequency = 1.0f;
    for (size_t i = 0; i < MaxOctaves; ++i)
    {
      exponentArray[i] = pow(frequency, -roughness * i);
      frequency *= lacunarity;
    }
  }

  octaves = glm::clamp(octaves, 0.0, (double)MaxOctaves-1);
  glm::dvec3 p = glm::dvec3(point);
  double result = 0.0;
  for (size_t i = 0; i < (size_t)octaves; ++i)
  {
    result += (glm::noise1(p) * exponentArray[i]);
    p *= lacunarity;
  }

  // deal with fractional part of 'octaves'...
  const double remainder = octaves - (int)octaves;
  if (remainder > 0)
  {
    result += (remainder * glm::noise1(p) * exponentArray[(int)octaves + 1]);
  }

  return result;
}

//-----------------------------------------------------------
static void InitialiseFace(TerrainTile& face, const glm::mat4& transform, float radius)
{
  face.size = radius * 2.0f;
  face.centre = glm::vec3(0, 0, 0);
  face.transform = transform;

  // Compute a height value for each position on the sphere by first transforming the global points
  // into world space, then normalizing to create the sphere and finally multiplying by the radius
  // to get the surface point...
  std::vector<float> heights(tileVertices.size());
  for (size_t i = 0; i < heights.size(); ++i)
  {
    const glm::vec3 P = radius * glm::vec3(glm::normalize(transform * glm::vec4(tileVertices[i].position, 1.0f)));
    const float h = (float)Fractal(P);
    heights[i] = h;
  }

  // Create a texture from the height data. We know that each tile is 65x65 posts wide...
  face.heightMap = boost::make_shared<Texture2D>();
  face.heightMap->Bind(Texture2D::MaxTextureUnit);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 65, 65, 0, GL_RED, GL_FLOAT, heights.data());
  face.heightMap->Unbind(Texture2D::MaxTextureUnit);

  glSamplerParameteri(face.heightMap->sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(face.heightMap->sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(face.heightMap->sampler, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glSamplerParameteri(face.heightMap->sampler, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}
