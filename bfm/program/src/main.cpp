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
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <cassert>

#include <logging.h>
#include <camera.h>
#include <timer.h>
#include <glbuffers.h>
#include <effect.h>
#include <terrain.h>

//-----------------------------------------------------------

FILE* logFile = NULL;

#define GL(func) do { func; GLenum glResult = glGetError(); if (GL_NO_ERROR != glResult) { LOG("gl error 0x%x\n", glResult); } } while (0)

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

static const size_t windowWidth = 1280;
static const size_t windowHeight = 720;

//-----------------------------------------------------------

static SDL_Window* window = NULL;
static SDL_GLContext glContext = NULL;
static bool closeProgram = false;
static Camera camera;

static boost::shared_ptr<Effect> displacement;

// Radius of planet (it's just a sphere located at (0,0,0))...
static float PlanetRadius = 100.0f;

// The sun is always off somewhere down the +X axis...
static glm::vec3 sunDirection(glm::vec3(1, 0, 0));

//-----------------------------------------------------------

static void OnExit();
static void Initialise();

static void PollEvents();
static void HandleInput();

//-----------------------------------------------------------
int main(int argc, char* argv[])
{
  Initialise();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  CameraInit(camera);
  camera.aspectRatio = (float)windowWidth / (float)windowHeight;
  // Position the camera at 3x the planet radius with the sun on the right...
  camera.position.z = PlanetRadius * 3.0f;

  displacement = boost::make_shared<Effect>();
  displacement->Load("assets\\effect.glsl", "GreenDisplacement");
  glProgramUniform1i(displacement->handle, displacement->params["HeightMapSampler"], 0);
  glProgramUniform1f(displacement->handle, displacement->params["PlanetRadius"], PlanetRadius);

  TerrainInitialise(PlanetRadius);

  unsigned int lastTime = 0;
  while (!closeProgram)
  {
    const unsigned int now = SDL_GetTicks();
    if (0 == lastTime) { lastTime = now; }
    const float elapsedMS = (float)(now - lastTime);
    lastTime = now;

    PollEvents();
    HandleInput();
    CameraUpdate(elapsedMS, camera);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(displacement->handle);

    TerrainDraw(camera.viewProjectionMatrix, *displacement);

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

  //if ((majorVersion < 4) || (minorVersion < 3))
  //{
  //  char msg[256];
  //  sprintf(msg, "OpenGL v4.3 or better needed (current is %d.%d)\nWill quit when message box closed.", majorVersion, minorVersion);
  //  LOG(msg);
  //  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Cannot run application", msg, NULL);
  //  exit(EXIT_FAILURE);
  //}

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
