#include <memory>
#include <core/keyboard.h>

//---------------------------------------------------------------------

void Keyboard::GetKeyState(KeyState* const state)
{
  const Uint8* keyState = SDL_GetKeyboardState(NULL);
  std::memcpy(state->keys, keyState, SDL_NUM_SCANCODES);
}

//---------------------------------------------------------------------

Keyboard::KeyState::KeyState()
{
  std::memset(keys, 0, SDL_NUM_SCANCODES);
}

//---------------------------------------------------------------------

Keyboard::KeyState::KeyState(const KeyState& other)
{
  std::memcpy(keys, other.keys, SDL_NUM_SCANCODES);
}

//---------------------------------------------------------------------

bool Keyboard::KeyState::KeyIsDown(SDL_Scancode code)
{
  return (0 != keys[code]);
}

//---------------------------------------------------------------------

bool Keyboard::KeyState::KeyIsUp(SDL_Scancode code)
{
  return (0 == keys[code]);
}
