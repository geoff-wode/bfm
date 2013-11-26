#include <boost/make_shared.hpp>

#include <core/logging.h>
#include <core/window.h>

#include "sdlattrs.h"

//------------------------------------------------------------------------

static void InitialiseSDL();
static void ReportSDLConfig();

//------------------------------------------------------------------------

Window::Window(
  const char* const title,
  const glm::ivec2& position,
  const glm::ivec2& size,
  bool fullScreen,
  bool visible)
  : position(position),
    size(size),
    fullScreen(fullScreen),
    visible(visible)
{
  MakeWindow(title);
}

//------------------------------------------------------------------------

Window::Window(
  const char* const title,
  const glm::ivec2& size,
  bool fullScreen,
  bool visible)
  : position(glm::ivec2(SDL_WINDOWPOS_CENTERED)),
    size(size),
    fullScreen(fullScreen),
    visible(visible)
{
  MakeWindow(title);
}

//------------------------------------------------------------------------

Window::~Window()
{
  if (glContext)
  {
    SDL_GL_DeleteContext(glContext);
  }

  if (window)
  {
    LOG("destroying window: %s\n", SDL_GetWindowTitle(window));
    SDL_DestroyWindow(window);
  }
}

//------------------------------------------------------------------------

void Window::Show()
{
  if (!visible)
  {
    SDL_ShowWindow(window);
    visible = true;
  }
}

//------------------------------------------------------------------------

void Window::Hide()
{
  if (visible)
  {
    SDL_HideWindow(window);
    visible = false;
  }
}


//------------------------------------------------------------------------

void Window::MakeWindow(const char* const title)
{
  InitialiseSDL();

  Uint32 flags = SDL_WINDOW_OPENGL;
  if (!visible) { flags |= SDL_WINDOW_HIDDEN; }
  if (fullScreen) { flags |= SDL_WINDOW_FULLSCREEN_DESKTOP; }
  window = SDL_CreateWindow(title, position.x, position.y, size.x, size.y, flags);

  if (window)
  {
    glContext = SDL_GL_CreateContext(window);
  }

  static bool firstTime = true;
  if (firstTime)
  {
    glInitLibrary();

    GLint majorVersion;
    GLint minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    LOG("GL version: %d.%d\n", majorVersion, minorVersion);
    LOG("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOG("Vendor: %s\n", glGetString(GL_VENDOR));
    LOG("Renderer: %s\n", glGetString(GL_RENDERER));

    firstTime = false;
  }

  context = boost::make_shared<Context>();

  SDL_GetWindowPosition(window, &position.x, &position.y);

  LOG("new window: %s @ (%d,%d) (%d x %d)\n", title, position.x, position.y, size.x, size.y);
  ReportSDLConfig();
}

//------------------------------------------------------------------------

void Window::EndFrame()
{
  SDL_GL_SwapWindow(window);
}

//------------------------------------------------------------------------

void Window::MakeCurrent()
{
  SDL_GL_MakeCurrent(window, glContext);
}

//------------------------------------------------------------------------

void Window::SetFullScreen(bool yes)
{
  if (yes != fullScreen)
  {
    SDL_SetWindowFullscreen(window, yes ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    fullScreen = yes;
  }
}

//------------------------------------------------------------------------

static void InitialiseSDL()
{
  for (int i = 0; i < NumSDLAttributes; ++i)
  {
    SDL_GL_SetAttribute(sdlAttributes[i].attr, sdlAttributes[i].value);
  }
}

//------------------------------------------------------------------------

static void ReportSDLConfig()
{
  for (int i = 0; i < NumSDLAttributes; ++i)
  {
    int value;
    SDL_GL_GetAttribute(sdlAttributes[i].attr, &value);
    LOG("%s = %d (requested %d)\n", sdlAttributes[i].name, value, sdlAttributes[i].value);
  }
}
