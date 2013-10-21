#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <logging.h>
#include <device.h>
#include <camera.h>
#include <utils.h>
#include <memory>
#include <terrain/planet.h>
#include <scenestate.h>
#include <boost/make_shared.hpp>

//-------------------------------------------------------------------

struct Scene
{
  boost::shared_ptr<Planet> planet;
  SceneState sceneState;
};

//-------------------------------------------------------------------

FILE* logFile;
static bool quit = false;
static ClearState clearState;
static Camera camera;
static Device device;

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

  if (keyState[SDL_SCANCODE_W]) { camera->movement.z -= 1; }
  if (keyState[SDL_SCANCODE_S]) { camera->movement.z += 1; }
  if (keyState[SDL_SCANCODE_A]) { camera->yaw -= 0.0001f; }
  if (keyState[SDL_SCANCODE_D]) { camera->yaw += 0.0001f; }
}

//-------------------------------------------------------------------
static void DrawScene(Scene& scene)
{
  scene.planet->Draw(scene.sceneState);
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

  static const float radius = 1000;
  scene.planet = boost::make_shared<Planet>(radius);
  if (!scene.planet->Initialise())
  {
    return EXIT_FAILURE;
  }

  camera.Initialise(device.BackbufferWidth, device.BackbufferHeight);
  camera.position = glm::dvec3(0, 0, 6000);
  camera.farClip = 10000;
  camera.nearClip = 10;
  camera.speed = 100;
  scene.sceneState.camera = &camera;

  // Constants controlling depth precision (smaller increases precision at distance at the
  // expense of loosing precision close-in.
  // See terrain.glsl for the gory details.
  static const float logDepthConstant = 1.0f;
  static const float logDepthOffset = 2.0f;
  static const float logDepthDivisor = (float)(1.0 / (glm::log(logDepthConstant * camera.farClip) + logDepthOffset));
  // The camera's far plane distance never changes so these values (used in logarithmic
  // depth buffer) can be set just once.
  scene.planet->effect.LogDepthConstant->Set(logDepthConstant);
  scene.planet->effect.LogDepthOffset->Set(logDepthOffset);
  scene.planet->effect.LogDepthDivisor->Set(logDepthDivisor);
  
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

    device.SwapBuffers();
  }

  return 0;
}
