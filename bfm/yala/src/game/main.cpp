#include <SDL.h>
#include <boost/make_shared.hpp>
#include <core/logging.h>
#include <core/device.h>
#include <core/keyboard.h>
#include <game/game.h>
#include <game/cameras/freecamera.h>
#include <game/planet/planet.h>

//------------------------------------------------------------------------

class MyGame : public Game
{
public:
  MyGame();
  virtual ~MyGame();

private:
  virtual void Initialise();
  virtual void Update(float elapsedMS);
  virtual void PreRender(float elapsedMS);
  virtual void Render(float elapsedMS);
  virtual void PostRender(float elapsedMS);
  virtual void Shutdown();

  void HandleInput();

  ClearState clearState;
  Keyboard::KeyState oldKeyState;
  FreeCamera camera;
  glm::dvec3 sunPosition;
  boost::shared_ptr<Planet> planet;
};

//------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  Logging::Initialise();

  MyGame game;
  game.Run();

  return 0;
}

//------------------------------------------------------------------------

MyGame::MyGame()
{
}

//------------------------------------------------------------------------

MyGame::~MyGame()
{
}

//------------------------------------------------------------------------

void MyGame::Initialise()
{
  // Create the game's main "view" and make it the current context for this thread's
  // GL calls. Start it off invisible so that the lengthy initialisation process isn't
  // quite so obvious.
  window = Device::NewWindow("stuff and nonsense", glm::ivec2(1280, 720), false, false);
  window->MakeCurrent();
  window->Show();

  Keyboard::GetKeyState(&oldKeyState);

  camera.aspectRatio = double(window->Size().x) / double(window->Size().y);

  planet = boost::make_shared<Planet>(6000);
  planet->Initialise();

  sunPosition = glm::dvec3(100000000, 0, 0);
}

//------------------------------------------------------------------------

void MyGame::Update(float elapsedMS)
{
  HandleInput();

  camera.Update(elapsedMS);
}

//------------------------------------------------------------------------

void MyGame::HandleInput()
{
  Keyboard::KeyState keyState;
  Keyboard::GetKeyState(&keyState);

  if (keyState.KeyIsDown(SDL_SCANCODE_ESCAPE))
  {
    Stop();
  }

  oldKeyState = keyState;
}

//------------------------------------------------------------------------

void MyGame::PreRender(float elapsedMS)
{
  window->context->Clear(clearState);
}

//------------------------------------------------------------------------

void MyGame::Render(float elapsedMS)
{
  const glm::vec3 sunDir = -glm::vec3(glm::normalize(sunPosition));
  planet->Draw(window->context, camera, sunDir);
}

//------------------------------------------------------------------------

void MyGame::PostRender(float elapsedMS)
{
}

//------------------------------------------------------------------------

void MyGame::Shutdown()
{
}
