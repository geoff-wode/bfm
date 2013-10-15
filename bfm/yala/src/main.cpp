#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <logging.h>
#include <device.h>
#include <camera.h>
#include <utils.h>
#include <memory>
#include <terraineffect.h>
#include <boost/make_shared.hpp>

//-------------------------------------------------------------------
// A vertex position is double-precision, encoded into 2 single-precision values.
struct Vertex
{
  glm::vec3 positionLow;
  glm::vec3 positionHigh;

  Vertex& operator=(const glm::dvec3& value)
  {
    DoubleToFloat(value, positionLow, positionHigh);
    return *this;
  }
};

//-------------------------------------------------------------------

static const size_t GridSize = 33;
static const size_t VertexCount = GridSize * GridSize;
static const size_t IndexCount = 2 * GridSize * (GridSize - 1);

//-------------------------------------------------------------------

struct TerrainTile
{
  size_t vertexStart;
  glm::dmat4 toWorld;
  VertexArray geometry;
};

//-------------------------------------------------------------------

FILE* logFile;
static bool quit = false;
static ClearState clearState;
static TerrainEffect effect;
static Camera camera;
static Device device;
static TerrainTile terrainTile;

//-------------------------------------------------------------------
// Build the geometry for a tile.
static void DefineGeometry(TerrainTile& tile)
{
  int x;
  int z;

  Vertex vertices[VertexCount];
  for (z = 0; z < GridSize; ++z)
  {
    for (x = 0; x < GridSize; ++x)
    {
      static const double offset = (double)GridSize / 2;

      const glm::dvec4 P = tile.toWorld * glm::dvec4(x - offset, z - offset, 0.0, 1.0);
      
      vertices[x + (z * GridSize)] = glm::dvec3(P);
    }
  }

  static boost::shared_ptr<IndexBuffer> indexBuffer;
  if (!indexBuffer)
  {
    unsigned short indices[IndexCount];
    int i = 0;
    z = 0;
    while (z < (GridSize - 1))
    {
      for (x = 0; x < GridSize; ++x)
      {
        indices[i++] = x + (z * GridSize);
        indices[i++] = x + ((z + 1) * GridSize);
      }
      ++z;
      if (z < (GridSize - 1))
      {
        for (x = GridSize - 1; x >= 0; --x)
        {
          indices[i++] = x + ((z + 1) * GridSize);
          indices[i++] = x + (z * GridSize);
        }
      }
      ++z;
    }
    indexBuffer = boost::make_shared<IndexBuffer>(IndexCount, GL_UNSIGNED_SHORT, GL_STATIC_DRAW, indices);
  }

  VertexLayout vertexLayout;
  vertexLayout.AddAttribute(VertexAttribute(VertexSemantic::PositionLow, GL_FLOAT_VEC3, offsetof(Vertex, positionLow)));
  vertexLayout.AddAttribute(VertexAttribute(VertexSemantic::PositionHigh, GL_FLOAT_VEC3, offsetof(Vertex, positionHigh)));

  boost::shared_ptr<VertexBuffer> vb(new VertexBuffer(vertexLayout, VertexCount, GL_STATIC_DRAW, vertices));

  tile.geometry.Initialise(vb, indexBuffer);
}

//-------------------------------------------------------------------
static void OnExit()
{
  fclose(logFile);
}

//-------------------------------------------------------------------
static void HandleInput(Camera& camera)
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT: quit = true; return;
    default: break;
    }
  }

  glm::ivec2 mousePos;
  const Uint32 buttons = SDL_GetRelativeMouseState(&mousePos.x, &mousePos.y);
  const Uint8* keyState = SDL_GetKeyboardState(NULL);

  if (keyState[SDL_SCANCODE_ESCAPE])
  {
    quit = true;
    return;
  }

  static const glm::vec2 mouseSensitivity = glm::vec2(0.01f, 0.005f);

  if (mousePos.x || mousePos.y)
  {
    camera.roll += (mouseSensitivity.x * (float)mousePos.x);
    camera.pitch -= (mouseSensitivity.y * (float)mousePos.y);
  }

  if (keyState[SDL_SCANCODE_W]) { camera.movement.z -= 1; }
  if (keyState[SDL_SCANCODE_S]) { camera.movement.z += 1; }
  if (keyState[SDL_SCANCODE_A]) { camera.yaw -= 0.0001; }
  if (keyState[SDL_SCANCODE_D]) { camera.yaw += 0.0001; }
}

//-------------------------------------------------------------------
static void DrawScene(RenderState renderState)
{
  glm::vec3 camPosLow;
  glm::vec3 camPosHigh;
  DoubleToFloat(camera.position, camPosLow, camPosHigh);
  effect.CameraPositionLow->Set(camPosLow);
  effect.CameraPositionHigh->Set(camPosHigh);

  glm::dmat4 rteModelView = camera.viewMatrix;
  rteModelView[3][0] = 0;
  rteModelView[3][1] = 0;
  rteModelView[3][2] = 0;
  const glm::dmat4 rteModelViewProjection = camera.projectionMatrix * rteModelView;

  effect.WorldViewProjectionMatrix->Set(glm::mat4(rteModelViewProjection));

  renderState.drawState.culling.windingOrder = GL_CW;
  renderState.vertexArray = &terrainTile.geometry;

  device.Draw(GL_TRIANGLE_STRIP, 0, IndexCount, renderState);
}

//-------------------------------------------------------------------
static void Update(double elapsedMS)
{
  HandleInput(camera);
  camera.Update(elapsedMS);
}

//-------------------------------------------------------------------
int main(int argc, char* argv[])
{
  logFile = fopen("stderr.txt", "wb");
  atexit(OnExit);

  device.Initialise(1280, 720, "");

  static const double terrainWidth = 6000; // KILOMETERS!

  camera.Initialise(device.BackbufferWidth, device.BackbufferHeight);
  camera.position = glm::dvec3(0, 0, terrainWidth * 2);
  camera.farClip = 100000000; // KILOMETERS!
  camera.nearClip = 1;

  effect.Load("effects\\terrain.glsl", "Terrain");

  terrainTile.vertexStart = 0;
  terrainTile.toWorld = glm::scale(glm::dvec3(terrainWidth));
  DefineGeometry(terrainTile);

  // Constant controlling depth precision (smaller increases precision at distance at the
  // expense of loosing precision close-in.
  // See terrain.glsl for the gory details.
  static const float logDepthConstant = 1.0f;
  static const float logDepthOffset = 2.0f;
  static const float logDepthDivisor = (float)(1.0 / (glm::log(logDepthConstant * camera.farClip) + logDepthOffset));

  // The camera's far plane distance never changes so these values (used in logarithmic
  // depth buffer) can be set just once.
  effect.LogDepthConstant->Set(logDepthConstant);
  effect.LogDepthOffset->Set(logDepthOffset);
  effect.LogDepthDivisor->Set(logDepthDivisor);

  RenderState renderState;
  renderState.drawState.polygonMode = GL_LINE;
  renderState.effect = &effect;

  unsigned int lastFrameTime = 0;
  while (!quit)
  {
    const unsigned int now = SDL_GetTicks();
    if (0 == lastFrameTime) { lastFrameTime = now; }
    const double elapsedMS = (double)(now - lastFrameTime);
    lastFrameTime = now;

    Update(elapsedMS);

    device.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, clearState);
    DrawScene(renderState);
    device.SwapBuffers();
  }

  return 0;
}
