#include <SDL.h>
#include <game/game.h>
#include <core/device.h>

//------------------------------------------------------------------------

Game::Game()
  : running(true)
{
  SDL_Init(SDL_INIT_EVERYTHING);
}

//------------------------------------------------------------------------

Game::~Game()
{
  SDL_Quit();
}

//------------------------------------------------------------------------

void Game::Run()
{
  Initialise();

  unsigned int lastTime = 0;
  while (running)
  {
    const unsigned int now = SDL_GetTicks();
    if (0 == lastTime) { lastTime  = now; }
    const float elapsedMS = float(now - lastTime);

    Update(elapsedMS);
    Render(elapsedMS);

    window->EndFrame();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (SDL_QUIT == event.type)
      {
        Stop();
      }
    }
  }

  Shutdown();
}

//------------------------------------------------------------------------

void Game::Stop()
{
  running = !AllowStop();
}

//------------------------------------------------------------------------
