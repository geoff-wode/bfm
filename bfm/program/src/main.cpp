#include <SDL.h>
#include <gl_loader/gl_loader.h>
#include <cstdio>
#include <cstdlib>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <glfx.h>
#include <map>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <cassert>

//-----------------------------------------------------------

static FILE* logFile = NULL;
#define LOG(msg, ...) do { fprintf(logFile, msg, __VA_ARGS__); } while (0)
#define ASSERT(pred) do { if (!(pred)) { LOG("assert failed: %s\n", #pred); assert(pred); } } while (0)

#define GL(func) do { func; GLenum glResult = glGetError(); if (GL_NO_ERROR != glResult) { LOG("gl error 0x%x\n", glResult); } } while (0)

#define SHADER_SEMANTIC_POSITION  0
#define SHADER_SEMANTIC_NORMAL    1
#define SHADER_SEMANTIC_TEXCOORD  2

//-----------------------------------------------------------
struct SDLAttribute
{
  SDL_GLattr  attr;
  int         value;
  const char* name;
};
#define SDL_ATTR(name, value) { name, value, #name }
static const SDLAttribute sdlAttributes[] =
{
  SDL_ATTR(SDL_GL_DOUBLEBUFFER,       1 ),
  SDL_ATTR(SDL_GL_RED_SIZE,           8 ),
  SDL_ATTR(SDL_GL_GREEN_SIZE,         8 ),
  SDL_ATTR(SDL_GL_BLUE_SIZE,          8 ),
  SDL_ATTR(SDL_GL_ALPHA_SIZE,         8 ),
  SDL_ATTR(SDL_GL_DEPTH_SIZE,         24),
  SDL_ATTR(SDL_GL_STENCIL_SIZE,       8 ),
  SDL_ATTR(SDL_GL_BUFFER_SIZE,        24),
  SDL_ATTR(SDL_GL_MULTISAMPLEBUFFERS, 1 ),
  SDL_ATTR(SDL_GL_MULTISAMPLESAMPLES, 2 )
};
const int NumSDLAttributes = sizeof(sdlAttributes)/sizeof(sdlAttributes[0]);

//-----------------------------------------------------------
struct Timer
{
  unsigned int elapsedMS;

  void Start() { start = SDL_GetTicks(); }
  void Stop() { elapsedMS = SDL_GetTicks() - start; }

private:
  unsigned int start;
};

//-----------------------------------------------------------
struct Vertex
{
  glm::vec3 position;
};

//-----------------------------------------------------------

struct Camera
{
  float fov;
  float nearClip;
  float farClip;
  float yaw;
  float pitch;
  float roll;
  float speed;
  glm::vec3 movement;
  glm::vec3 position;
  glm::vec3 up;
  glm::vec3 forward;
  glm::vec3 right;
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
};

//-----------------------------------------------------------

struct GLBuffer
{
  GLuint handle;

  GLBuffer() { glGenBuffers(1, &handle); }
  ~GLBuffer() { glDeleteBuffers(1, &handle); }
};

//-----------------------------------------------------------
struct VAO
{
  GLuint handle;
  size_t indexCount;

  boost::shared_ptr<GLBuffer> vertexBuffer;
  boost::shared_ptr<GLBuffer> indexBuffer;

  VAO() { glGenVertexArrays(1, &handle); }
  ~VAO() { glDeleteVertexArrays(1, &handle); }
};

//-----------------------------------------------------------
struct Effect
{
  GLuint handle;
  std::map<std::string, GLint> params;

  Effect() { }
  ~Effect() { glDeleteProgram(handle); }
};

//-----------------------------------------------------------
struct Texture2D
{
  static const size_t MaxTextureUnit = 31;
  GLuint handle;
  GLuint sampler;

  Texture2D()
  {
    glGenTextures(1, &handle);
    glGenSamplers(1, &sampler);
  }
  
  ~Texture2D()
  {
    glDeleteSamplers(1, &sampler);
    glDeleteTextures(1, &handle);
  }

  void Bind(size_t textureUnit)
  {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindSampler(textureUnit, sampler);
    glBindTexture(GL_TEXTURE_2D, handle);
  }

  void Unbind(size_t textureUnit)
  {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindSampler(textureUnit, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE31);
  }
};

//-----------------------------------------------------------
struct Tile
{
  glm::mat4 transform;
  boost::shared_ptr<VAO> vao;
  boost::shared_ptr<Texture2D> heightMap;

  static boost::shared_ptr<std::vector<Vertex>> vertices;
  static boost::shared_ptr<std::vector<GLshort>> indices;

  Tile(float size, const glm::vec3& position)
  {
    transform = glm::translate(position) * glm::scale(glm::vec3(size));
  }

  void Draw()
  {
    heightMap->Bind(0);
    glBindVertexArray(vao->handle);
    glDrawElements(GL_TRIANGLES, vao->indexCount, GL_UNSIGNED_SHORT, 0);
  }
};

boost::shared_ptr<std::vector<Vertex>> Tile::vertices;
boost::shared_ptr<std::vector<GLshort>> Tile::indices;

//-----------------------------------------------------------

static const size_t windowWidth = 1280;
static const size_t windowHeight = 720;
static const float aspectRatio = (float)windowWidth / (float)windowHeight;

//-----------------------------------------------------------

static SDL_Window* window = NULL;
static SDL_GLContext glContext = NULL;
static bool closeProgram = false;
static Camera camera;
static boost::shared_ptr<Effect> green;
static boost::shared_ptr<Effect> displacement;

static float PlanetRadius = 100.0f;
static glm::dvec3 planetPos(glm::sphericalRand(149600000000.0f));
static glm::vec3 sunDirection(glm::normalize(-planetPos));

//-----------------------------------------------------------

static void OnExit();
static void Initialise();

static void PollEvents();
static void HandleInput();
static void UpdateCamera(float elapsedMS, Camera& camera);

static boost::shared_ptr<Effect> LoadEffect(const char* const file, const char* const programName);
static void LoadEffectParams(GLuint effect, std::map<std::string,GLint>& params);

static boost::shared_ptr<Texture2D> CreateHeightMapTexture(boost::shared_ptr<Tile> tile);

static void LoadGridModel();
static boost::shared_ptr<VAO> CreateGrid(const std::vector<Vertex>* vertices, const std::vector<GLshort>* indices);

// Return a fractal value for the location of 'point'.
static float Fractal(
  const glm::vec3& point,
  float octaves = 4,
  float lacunarity = 2.0123f,
  float roughness = 0.679f,
  float offset = 0.7f);

//-----------------------------------------------------------
int main(int argc, char* argv[])
{
  Timer timer;

  Initialise();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  displacement = LoadEffect("assets\\effect.glsl", "GreenDisplacement");
  glProgramUniform1i(displacement->handle, displacement->params["HeightMapSampler"], 0);

  boost::shared_ptr<Tile> tile(new Tile(PlanetRadius, glm::vec3(0)));
  timer.Start();
  LoadGridModel();
  timer.Stop();
  tile->vao = CreateGrid(Tile::vertices.get(), Tile::indices.get());
  LOG("loaded tile model: %fs\n", (float)timer.elapsedMS / 1000.0f);
  timer.Start();
  tile->heightMap = CreateHeightMapTexture(tile);
  timer.Stop();
  LOG("gen height map: %fs\n", (float)timer.elapsedMS / 1000.0f);

  unsigned int lastTime = 0;
  while (!closeProgram)
  {
    const unsigned int now = SDL_GetTicks();
    if (0 == lastTime) { lastTime = now; }
    const float elapsedMS = (float)(now - lastTime);
    lastTime = now;

    PollEvents();
    HandleInput();
    UpdateCamera(elapsedMS, camera);

    const glm::mat4 viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
    const glm::mat4 worldViewProjectionMatrix = viewProjectionMatrix * glm::mat4();
    const glm::mat3 normalMatrix = glm::mat3();

    glProgramUniformMatrix4fv(displacement->handle, displacement->params["WorldViewProjectionMatrix"], 1, GL_FALSE, glm::value_ptr(worldViewProjectionMatrix));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(displacement->handle);

    tile->Draw();

    SDL_GL_SwapWindow(window);
  }

  return 0;
}

//-----------------------------------------------------------

static void OnExit()
{
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  fclose(logFile);
  SDL_Quit();
}

//-----------------------------------------------------------

static void Initialise()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  logFile = fopen("stderr.txt", "wb");
  atexit(OnExit);

  for (int i = 0; i < NumSDLAttributes; ++i) { SDL_GL_SetAttribute(sdlAttributes[i].attr, sdlAttributes[i].value); }

  window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
  glContext = SDL_GL_CreateContext(window);
  
  for (int i = 0; i < NumSDLAttributes; ++i) { SDL_GL_SetAttribute(sdlAttributes[i].attr, sdlAttributes[i].value); }
  
  for (int i = 0; i < NumSDLAttributes; ++i)
  {
    int value;
    SDL_GL_GetAttribute(sdlAttributes[i].attr, &value);
    LOG("%s = %d (requested %d)\n", sdlAttributes[i].name, value, sdlAttributes[i].value);
  }

  glInitLibrary();
  GLint majorVersion;
  GLint minorVersion;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
  LOG("GL version: %d.%d\n", majorVersion, minorVersion);

  if ((majorVersion < 4) || (minorVersion < 3))
  {
    char msg[256];
    sprintf(msg, "OpenGL v4.3 or better needed (current is %d.%d)\nWill quit when message box closed.", majorVersion, minorVersion);
    LOG(msg);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Cannot run application", msg, NULL);
    exit(EXIT_FAILURE);
  }

  LOG("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  LOG("Vendor: %s\n", glGetString(GL_VENDOR));
  LOG("Renderer: %s\n", glGetString(GL_RENDERER));

  glClearColor(0, 0, 0, 0);
  glClearDepth(1.0f);
  
  glColorMask(true, true, true, true);
  glDepthMask(true);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  camera.fov = 45.0f;
  camera.nearClip = 0.1f;
  camera.farClip = 1000.0f;
  camera.yaw = 0;
  camera.pitch = 70.0f;
  camera.roll = 0;
  camera.speed = 5;
  camera.movement = glm::vec3(0);
  camera.position = glm::vec3(0, 100, 10);
  camera.up = glm::vec3(0,1,0);
  camera.forward = glm::vec3(0,0,-1);
  camera.right = glm::vec3(1,0,0);
  camera.viewMatrix = glm::mat4();
  camera.projectionMatrix = glm::mat4();

  SDL_SetRelativeMouseMode(SDL_TRUE);
}

//-----------------------------------------------------------
static void PollEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT: closeProgram = true; break;
    default: break;
    }
  }
}

//-----------------------------------------------------------
static void UpdateCamera(float elapsedMS, Camera& camera)
{
  // Determine where the local XYZ axes are pointing...
  const glm::mat4 orientation = glm::yawPitchRoll(camera.yaw, camera.pitch, camera.roll);
  const glm::mat4 invOrientation(glm::inverse(orientation));
  camera.up = (glm::vec3)(invOrientation * glm::vec4(0,1,0,0));
  camera.forward = (glm::vec3)(invOrientation * glm::vec4(0,0,-1,0));
  camera.right = (glm::vec3)(invOrientation * glm::vec4(1,0,0,0));

  // Move along the local XYZ axes...
  const float elapsedSeconds = elapsedMS / 1000.0f;
  const float velocity(elapsedSeconds * camera.speed);
  camera.position += (glm::vec3)(velocity * glm::vec4(camera.movement,0) * orientation);
  camera.movement = glm::vec3(0);

  // Compute new matrices...
  camera.viewMatrix = orientation * glm::translate(-camera.position);
  camera.projectionMatrix = glm::perspective(camera.fov, aspectRatio, camera.nearClip, camera.farClip);
}

//-----------------------------------------------------------
static void HandleInput()
{
  glm::ivec2 mousePos;
  const Uint32 buttons = SDL_GetRelativeMouseState(&mousePos.x, &mousePos.y);
  const Uint8* keyState = SDL_GetKeyboardState(NULL);

  if (keyState[SDL_SCANCODE_ESCAPE])
  {
    closeProgram = true;
    return;
  }

  if (mousePos.x || mousePos.y)
  {
    const glm::vec2 mouseSensitivity = glm::vec2(0.01f, 0.005f);
    camera.roll += (mouseSensitivity.x * (float)mousePos.x);
    camera.pitch -= (mouseSensitivity.y * (float)mousePos.y);
  }

  if (keyState[SDL_SCANCODE_W]) { camera.movement.z -= 1; }
  if (keyState[SDL_SCANCODE_A]) { camera.movement.x -= 1; }
  if (keyState[SDL_SCANCODE_S]) { camera.movement.z += 1; }
  if (keyState[SDL_SCANCODE_D]) { camera.movement.x += 1; }
}

//-----------------------------------------------------------
static void LoadGridModel()
{
  if (!Tile::vertices)
  {
    Tile::vertices = boost::make_shared<std::vector<Vertex>>();
    Tile::indices = boost::make_shared<std::vector<GLshort>>();
  }

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
    Tile::vertices->push_back(v);
  }

  for (size_t i = 0; i < mesh->mNumFaces; ++i)
  {
    const aiFace& face = mesh->mFaces[i];
    Tile::indices->push_back(face.mIndices[0]);
    Tile::indices->push_back(face.mIndices[1]);
    Tile::indices->push_back(face.mIndices[2]);
  }
  ASSERT(Tile::indices->size() <= USHRT_MAX);
}

//-----------------------------------------------------------
static boost::shared_ptr<VAO> CreateGrid(const std::vector<Vertex>* vertices, const std::vector<GLshort>* indices)
{
  boost::shared_ptr<VAO> grid = boost::make_shared<VAO>();
  glBindVertexArray(grid->handle);
  grid->indexCount = indices->size();
  grid->vertexBuffer = boost::make_shared<GLBuffer>();
  grid->indexBuffer = boost::make_shared<GLBuffer>();

  glBindBuffer(GL_ARRAY_BUFFER, grid->vertexBuffer->handle);
  glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(Vertex), vertices->data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid->indexBuffer->handle);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLushort), indices->data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(SHADER_SEMANTIC_POSITION);
  glVertexAttribPointer(SHADER_SEMANTIC_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  return grid;
}

//-----------------------------------------------------------
static boost::shared_ptr<Effect> LoadEffect(const char* const file, const char* const programName)
{
  boost::shared_ptr<Effect> effect;

  const int glfx = glfxGenEffect();
  if (glfxParseEffectFromFile(glfx, file))
  {
    effect = boost::make_shared<Effect>();
    effect->handle = glfxCompileProgram(glfx, programName);
    if (-1 != effect->handle)
    {
      LoadEffectParams(effect->handle, effect->params);
      LOG("effect %s::%s compiled - uniform params:\n", file, programName);
      BOOST_FOREACH(auto v, effect->params)
      {
        LOG("  %s\n", v.first.c_str());
      }
    }
    else
    {
      LOG("\n%s::%s\n%s\n", file, programName, glfxGetEffectLog(glfx));
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, programName, glfxGetEffectLog(glfx), NULL);
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    LOG("\ncould not parse %s::%s:\n%s\n", file, programName, glfxGetEffectLog(glfx));
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, programName, glfxGetEffectLog(glfx), NULL);
    exit(EXIT_FAILURE);
  }

  glfxDeleteEffect(glfx);
  return effect;
}

//-----------------------------------------------------------
static void LoadEffectParams(GLuint effect, std::map<std::string,GLint>& params)
{
	GLint numUniforms;
	glGetProgramiv(effect, GL_ACTIVE_UNIFORMS, &numUniforms);

	std::vector<GLuint> indices(numUniforms);
	for (GLint i = 0; i < numUniforms; ++i) { indices[i] = i; }

	std::vector<GLint> types(numUniforms);
	glGetActiveUniformsiv(effect, numUniforms, indices.data(), GL_UNIFORM_TYPE, types.data());

	std::vector<GLint> blockIndices(numUniforms);
	glGetActiveUniformsiv(effect, numUniforms, indices.data(), GL_UNIFORM_BLOCK_INDEX, blockIndices.data());

	for (int i = 0; i < numUniforms; ++i)
	{
		if (-1 == blockIndices[i])
		{
			// not a named uniform block...
			char uniformName[256] = { 0 };
			glGetActiveUniformName(effect, i, sizeof(uniformName)-1, NULL, uniformName);
			params[uniformName] = glGetUniformLocation(effect, uniformName);
		}
	}
}

//-----------------------------------------------------------
static boost::shared_ptr<Texture2D> CreateHeightMapTexture(boost::shared_ptr<Tile> tile)
{
  const int width = glm::sqrt(tile->vertices->size());
  const glm::vec3 base = glm::vec3(-width / 2.0f, 0.0f, width / 2.0f);

  boost::shared_ptr<Texture2D> texture(new Texture2D());

  std::vector<float> elevations(tile->vertices->size());

  for (int x = 0; x < width; ++x)
  {
    for (int y = 0; y < width; ++y)
    {
      const GLshort index = tile->indices->at(x + (y * width));
      const Vertex& v = tile->vertices->at(index);

      glm::vec4 p = glm::vec4(glm::normalize(v.position) * PlanetRadius, 1);
      p = tile->transform * p;

      float h = Fractal(p.xyz());
      elevations[x + (y * width)] = h;
    }
  }

  texture->Bind(Texture2D::MaxTextureUnit);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, width, 0, GL_RED, GL_FLOAT, elevations.data());

  glSamplerParameteri(texture->sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(texture->sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(texture->sampler, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glSamplerParameteri(texture->sampler, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  texture->Unbind(Texture2D::MaxTextureUnit);

  return texture;
}

//-----------------------------------------------------------
// Return a fractal value for the location of 'point'.
static float Fractal(
  const glm::vec3& point,
  float octaves,
  float lacunarity,
  float roughness,
  float offset)
{
  static const float MaxOctaves = 16.0f;
  static boost::shared_ptr<std::vector<float>> exponentArray;
  static bool first = true;

  octaves = glm::clamp(octaves, 0.0f, MaxOctaves);

  //if (first)
  //{
  //  // Pre-compute the exponents for speed...
  //  first = false;
  //  exponentArray = boost::make_shared<std::vector<float>>((int)MaxOctaves + 1);
  //  float frequency = 1.0f;
  //  for (size_t i = 0; i < exponentArray->size(); ++i)
  //  {
  //    exponentArray->at(i) = pow(frequency, -roughness);
  //    frequency *= lacunarity;
  //  }
  //}

  glm::vec3 p = point;
  float result = 0.0f;
  for (size_t i = 0; i < (size_t)octaves; ++i)
  {
    result += (glm::noise1(p) * pow(lacunarity, -roughness*i));
    p *= lacunarity;
  }

  // deal with fractional part of 'octaves'...
  const float remainder = octaves - (int)octaves;
  if (remainder > 0.0f)
  {
    result += (remainder * glm::noise1(p) * pow(lacunarity, -roughness * ((int)octaves + 1)));
  }

  return result;
}
