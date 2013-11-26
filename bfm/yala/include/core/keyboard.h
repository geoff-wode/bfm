#if ! defined(__KEYBOARD__)
#define __KEYBOARD__

#include <SDL.h>
#include <vector>

namespace Keyboard
{
  struct KeyState
  {
    KeyState();
    KeyState(const KeyState& other);

    Uint8 keys[SDL_NUM_SCANCODES];

    bool KeyIsDown(SDL_Scancode code);
    bool KeyIsUp(SDL_Scancode code);
  };

  void GetKeyState(KeyState* const state);
}

#endif // __KEYBOARD__
