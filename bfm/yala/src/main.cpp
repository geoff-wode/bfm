#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <logging.h>
#include <device.h>
#include <camera.h>
#include <utils.h>
#include <memory>
#include <terrain/terrain.h>
#include <textrenderer/font.h>
#include <scenestate.h>
#include <boost/make_shared.hpp>

//-------------------------------------------------------------------

struct Scene
{
  Terrain* terrain;
  SceneState sceneState;
};

//-------------------------------------------------------------------

FILE* logFile;
static bool quit = false;
static ClearState clearState;
static Camera camera;
static Device device;
static FontManager fontManager;

//-------------------------------------------------------------------

static void OnExit()
{
  fclose(logFile);
}

//-------------------------------------------------------------------
static void HandleInput(Camera* const camera)
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
    camera->roll += (mouseSensitivity.x * (float)mousePos.x);
    camera->pitch -= (mouseSensitivity.y * (float)mousePos.y);
  }

  if (keyState[SDL_SCANCODE_Z]) { camera->movement.y -= 1; }
  if (keyState[SDL_SCANCODE_X]) { camera->movement.y += 1; }

  if (keyState[SDL_SCANCODE_W]) { camera->movement.z -= 1; }
  if (keyState[SDL_SCANCODE_S]) { camera->movement.z += 1; }
  if (keyState[SDL_SCANCODE_A]) { camera->yaw -= 0.01f; }
  if (keyState[SDL_SCANCODE_D]) { camera->yaw += 0.01f; }
}

//-------------------------------------------------------------------
static void DrawScene(Scene& scene)
{
  scene.terrain->Draw(scene.sceneState);
}

//-------------------------------------------------------------------
static void Update(SceneState& sceneState)
{
  HandleInput(sceneState.camera);
  sceneState.camera->Update(sceneState.elapsedMS);
}

//-------------------------------------------------------------------
int main(int argc, char* argv[])
{
  logFile = fopen("stderr.txt", "wb");
  atexit(OnExit);

  Scene scene;

  device.Initialise(1280, 720, "");
  scene.sceneState.device = &device;

  fontManager.Initialise(glm::ivec2(device.BackbufferWidth, device.BackbufferHeight));
  boost::shared_ptr<Font> font = fontManager.LoadFont("assets\\fonts\\01DigiGraphics@12");

  static const float width = 10000;
  // Compute height range from 20 metres below sea-level to a maximum of some
  // fraction of the width.
  Terrain terrain(width, glm::vec2(-20, 10000));
  if (!terrain.Initialise())
  {
    return EXIT_FAILURE;
  }

  scene.terrain = &terrain;

  camera.Initialise(device.BackbufferWidth, device.BackbufferHeight);
  camera.position = glm::vec3(0, 1000, 0);
  camera.farClip = 10000000;
  camera.nearClip = 0.1f;
  camera.speed = 500.0f;
  scene.sceneState.camera = &camera;

  // Constants controlling depth precision (smaller increases precision at distance at the
  // expense of loosing precision close-in.
  // See terrain.glsl for the gory details.
  static const float logDepthConstant = 1.0f;
  static const float logDepthOffset = 2.0f;
  static const float logDepthDivisor = (float)(1.0 / (glm::log(logDepthConstant * camera.farClip) + logDepthOffset));
  // The camera's far plane distance never changes so these values (used in logarithmic
  // depth buffer) can be set just once.
  scene.terrain->effect.LogDepthConstant->Set(logDepthConstant);
  scene.terrain->effect.LogDepthOffset->Set(logDepthOffset);
  scene.terrain->effect.LogDepthDivisor->Set(logDepthDivisor);
  
  SceneState sceneState;
  sceneState.camera = &camera;
  sceneState.device = &device;

  unsigned int lastFrameTime = 0;
  while (!quit)
  {
    const unsigned int now = SDL_GetTicks();
    if (0 == lastFrameTime) { lastFrameTime = now; }
    sceneState.elapsedMS = (float)(now - lastFrameTime);
    lastFrameTime = now;

    Update(sceneState);

    device.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, clearState);

    DrawScene(scene);

    fontManager.DrawText(font, &device, "abcdefghijklmnopqrstuvwxyz", glm::ivec2(100, 500), glm::vec4(1,0,0,1));

    device.SwapBuffers();
  }

  return 0;
}
