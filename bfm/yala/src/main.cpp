#include <SDL.h>
#include <device.h>
#include <logging.h>
#include <camera.h>
#include <indexbuffer.h>
#include <vertexbuffer.h>
#include <vertexarray.h>

//-------------------------------------------------------------------

static void PollEvents();
static void HandleInput(Camera& camera);
static boost::shared_ptr<VertexArray> BuildVertexArray();

//-------------------------------------------------------------------

FILE* logFile;

//-------------------------------------------------------------------

static const size_t GridSize = 5;

static bool quit = false;
static ClearState clearState;
static RenderState renderState;

//-------------------------------------------------------------------
int main(int argc, char* argv[])
{
  logFile = fopen("stderr.txt", "wb");

  Device device;
  device.BackbufferWidth = 1280;
  device.BackbufferHeight = 720;

  if (!device.Initialise("YALA")) { return 0; }
  SDL_SetRelativeMouseMode(SDL_TRUE);

  Effect effect;
  effect.Load("effects\\rendergeometry.glsl", "RenderGeometry");

  Camera camera(device.BackbufferWidth, device.BackbufferHeight);
  camera.position = glm::vec3(0,10,10);

  boost::shared_ptr<VertexArray> vertexArray = BuildVertexArray();

  renderState.vertexArray = vertexArray.get();
  renderState.effect = &effect;

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glm::mat4 worldTransform = glm::mat4(1);

  unsigned int lastTime = 0;
  while (!quit)
  {
    PollEvents();

    if (!quit)
    {
      const unsigned int now = SDL_GetTicks();
      if (0 == lastTime) { lastTime = now; }
      const float elapsedMS = (float)(now - lastTime);
      lastTime = now;

      // update game state
      {
        HandleInput(camera);
        camera.Update(elapsedMS);
      }

      // render
      {
        device.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, clearState);
      
        effect.WorldViewProjectionMatrix.Set(camera.viewProjectionMatrix * worldTransform);

        renderState.drawState.culling.windingOrder = GL_CW;

        device.Draw(GL_TRIANGLE_STRIP, 0, vertexArray->GetIndexBuffer()->GetIndexCount(), renderState);

        device.SwapBuffers();
      }
    }
  }

  fclose(logFile);

  return 0;
}

//-------------------------------------------------------------------
static void PollEvents()
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
}

//-----------------------------------------------------------
static void HandleInput(Camera& camera)
{
  glm::ivec2 mousePos;
  const Uint32 buttons = SDL_GetRelativeMouseState(&mousePos.x, &mousePos.y);
  const Uint8* keyState = SDL_GetKeyboardState(NULL);

  if (keyState[SDL_SCANCODE_ESCAPE])
  {
    quit = true;
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
static boost::shared_ptr<VertexArray> BuildVertexArray()
{
  struct Vertex
  {
    glm::vec3 position;
  };

  VertexLayout vertexLayout;
  vertexLayout.AddAttribute(VertexAttribute(VertexSemantic::Position, GL_FLOAT_VEC3, offsetof(Vertex,position)));
  
  static const size_t numVertices = GridSize * GridSize;
  static const size_t numIndices = 2 * GridSize * (GridSize - 1);

  Vertex vertices[numVertices];
  {
    for (int z = 0; z < GridSize; ++z)
    {
      for (int x = 0; x < GridSize; ++x)
      {
        static const float offset = (float)GridSize / 2;
        vertices[x + (z * GridSize)].position = glm::vec3((float)x, 0.0f, (float)-z);
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
