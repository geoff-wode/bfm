#if ! defined(__GAME__)
#define __GAME__

#include <core/window.h>
#include <core/scenestate.h>

// Main entry point to the program.
// Overload the methods.
class Game
{
public:
  virtual ~Game();

  // Start the game loop.
  void Run();

  // End the game loop.
  void Stop();

protected:
  Game();

  // Main rendering window.
  boost::shared_ptr<Window> window;

  // Global scene state.
  SceneState sceneState;

private:
  // Called before the game loop is entered.
  // Games should, at the very least, create a window and make it current.
  virtual void Initialise() { }

  // Called within the main game loop to update the game's internal logic.
  virtual void Update(float elapsedMS) { }

  // Called within the main game loop to render the game to screen.
  virtual void Render(float elapsedMS) { }

  // Called when Stop is called.
  // Return true to allow the game to shutdown, or false to ignore the stop.
  virtual bool AllowStop() { return true; }

  // Called when the game loop is ended to allow the game to clean up any
  // allocated resources.
  virtual void Shutdown() { }

  bool running;
};

#endif // __GAME__
