#include <SDL.h>
#include <cmath>
#include <device.h>
#include <utils.h>
#include <logging.h>
#include <camera.h>
#include <indexbuffer.h>
#include <vertexbuffer.h>
#include <vertexarray.h>
#include <framebuffer.h>
#include <rterendergeometryeffect.h>
#include <boost/make_shared.hpp>

//-------------------------------------------------------------------

struct Context
{
  boost::shared_ptr<Device> device;
  boost::shared_ptr<Camera> camera;
};

//-------------------------------------------------------------------

static void HandleInput();

static boost::shared_ptr<VertexArray> BuildVertexArray();

static void GeometryPass(Context* const context);
static void LightingPass(Context* const context);

//-------------------------------------------------------------------

FILE* logFile;

//-------------------------------------------------------------------

static const size_t GridSize = 33;

static bool quit = false;
static ClearState clearState;
static RenderState renderState;
static Context context;

//-------------------------------------------------------------------
int main(int argc, char* argv[])
{
  logFile = fopen("stderr.txt", "wb");

  context.device = boost::make_shared<Device>();
  context.device->BackbufferWidth = 1280;
  context.device->BackbufferHeight = 720;

  if (!context.device->Initialise("YALA")) { return 0; }
  SDL_SetRelativeMouseMode(SDL_TRUE);

  RTERenderGeometryEffect effect;
  effect.Load("effects\\rendergeometry.glsl", "RenderGeometry");

  context.camera = boost::make_shared<Camera>(context.device->BackbufferWidth, context.device->BackbufferHeight);
  context.camera->position = glm::dvec3(0,10,10);
  context.camera->farClip = 10000000;
  context.camera->nearClip = 1;

  boost::shared_ptr<VertexArray> vertexArray = BuildVertexArray();

  renderState.vertexArray = vertexArray.get();
  renderState.effect = &effect;

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glm::dmat4 worldTransform = glm::dmat4(1);

  unsigned int lastTime = 0;
  while (!quit)
  {
    const unsigned int now = SDL_GetTicks();
    if (0 == lastTime) { lastTime = now; }
    const float elapsedMS = (float)(now - lastTime);
    lastTime = now;

    // update game state
    HandleInput();
    if (!quit)
    {
      context.camera->Update(elapsedMS);

      // render
      {
        context.device->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, clearState);
      
        // Create a model-view matrix and set its translation to (0,0,0) before using it to
        // create the full MVP matrix...
        glm::dmat4 modelViewMatrix = context.camera->viewMatrix * worldTransform;
        modelViewMatrix[3][0] = 0.0;
        modelViewMatrix[3][1] = 0.0;
        modelViewMatrix[3][2] = 0.0;
        
        glm::vec3 camPosLow;
        glm::vec3 camPosHigh;
        DoubleToFloat(context.camera->position, camPosLow, camPosHigh);
        effect.CameraPositionLow->Set(camPosLow);
        effect.CameraPositionHigh->Set(camPosHigh);

        effect.FarClipPlaneDistance->Set((float)context.camera->farClip);

        effect.RTEWorldViewProjectionMatrix->Set(glm::mat4(context.camera->projectionMatrix * modelViewMatrix));

        renderState.drawState.culling.windingOrder = GL_CW;

        context.device->Draw(GL_TRIANGLE_STRIP, 0, vertexArray->GetIndexBuffer()->GetIndexCount(), renderState);

        context.device->SwapBuffers();
      }
    }
  }

  fclose(logFile);

  return 0;
}

//-----------------------------------------------------------
static void HandleInput()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT: quit = true; break;
    default: break;
    }
  }

  if (!quit)
  {
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
      context.camera->roll += (mouseSensitivity.x * (float)mousePos.x);
      context.camera->pitch -= (mouseSensitivity.y * (float)mousePos.y);
    }

    if (keyState[SDL_SCANCODE_W]) { context.camera->movement.z -= 1; }
    if (keyState[SDL_SCANCODE_S]) { context.camera->movement.z += 1; }
    if (keyState[SDL_SCANCODE_A]) { context.camera->yaw -= 0.0001; }
    if (keyState[SDL_SCANCODE_D]) { context.camera->yaw += 0.0001; }
  }
}

//-----------------------------------------------------------
static boost::shared_ptr<VertexArray> BuildVertexArray()
{
  struct Vertex
  {
    glm::vec3 positionLow;
    glm::vec3 positionHigh;
  };

  VertexLayout vertexLayout;
  vertexLayout.AddAttribute(VertexAttribute(VertexSemantic::PositionLow, GL_FLOAT_VEC3, offsetof(Vertex,positionLow)));
  vertexLayout.AddAttribute(VertexAttribute(VertexSemantic::PositionHigh, GL_FLOAT_VEC3, offsetof(Vertex,positionHigh)));
  
  static const size_t numVertices = GridSize * GridSize;
  static const size_t numIndices = 2 * GridSize * (GridSize - 1);

  Vertex vertices[numVertices];
  {
    for (int z = 0; z < GridSize; ++z)
    {
      for (int x = 0; x < GridSize; ++x)
      {
        static const float offset = (float)GridSize / 2;
        const glm::dvec3 position = glm::dvec3((double)x, 0.0f, (double)-z);
        Vertex& vertex = vertices[x + (z * GridSize)];
        DoubleToFloat(position, vertex.positionLow, vertex.positionHigh);
      }
    }
  }
  boost::shared_ptr<VertexBuffer> vertexBuffer(new VertexBuffer(vertexLayout, numVertices, GL_STATIC_DRAW, vertices));

  unsigned short indices[numIndices];
  int i = 0;
  int z = 0;
  while (z < (GridSize - 1))
  {
    for (int x = 0; x < GridSize; ++x)
    {
      indices[i++] = x + (z * GridSize);
      indices[i++] = x + ((z + 1) * GridSize);
    }
    ++z;
    if (z < (GridSize - 1))
    {
      for (int x = GridSize - 1; x >= 0; --x)
      {
        indices[i++] = x + ((z + 1) * GridSize);
        indices[i++] = x + (z * GridSize);
      }
    }
    ++z;
  }
  boost::shared_ptr<IndexBuffer> indexBuffer(new IndexBuffer(numIndices, GL_UNSIGNED_SHORT, GL_STATIC_DRAW, indices));

  boost::shared_ptr<VertexArray> vertexArray(new VertexArray(vertexBuffer, indexBuffer));
  return vertexArray;
}
