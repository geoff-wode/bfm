#if ! defined(__SDL_ATTRS__)
#define __SDL_ATTRS__

#include <SDL.h>

//------------------------------------------------------------------------

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
  SDL_ATTR(SDL_GL_MULTISAMPLESAMPLES, 4 )
};
const int NumSDLAttributes = sizeof(sdlAttributes)/sizeof(sdlAttributes[0]);

//------------------------------------------------------------------------

#endif // __SDL_ATTRS__
