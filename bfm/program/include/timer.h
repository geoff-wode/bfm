#if ! defined(__TIMER__)
#define __TIMER__

#include <SDL.h>

struct Timer
{
  unsigned int elapsedMS;

  void Start() { start = SDL_GetTicks(); }
  void Stop() { elapsedMS = SDL_GetTicks() - start; }

private:
  unsigned int start;
};

#endif // __TIMER__
