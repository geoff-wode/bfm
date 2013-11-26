#if ! defined(__WINDOW__)
#define __WINDOW__

#include <SDL.h>
#include <glm/glm.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <core/context.h>

// Each window contains its own GL context and so can be executed in its own thread.
class Window : public boost::noncopyable
{
public:
  // Construct a new OpenGL-enabled window.
  Window(
    const char* const title,
    const glm::ivec2& position,
    const glm::ivec2& size,
    bool fullScreen,
    bool visible);

  // Construct a new OpenGL-enabled window positioned at the centre of the primary display.
  Window(
    const char* const title,
    const glm::ivec2& size,
    bool fullScreen,
    bool visible);

  ~Window();

  void Show();
  void Hide();
  void SetFullScreen(bool yes);

  // MakeCurrent must be called before any GL calls are made in the window's context.
  // This allows for multiple threads to operate independently when e.g. creating
  // textures or compiling shaders.
  void MakeCurrent();

  // Cause the window to flip the back- and front-buffers.
  // It is called automatically by the main Game loop but is not necessary if the
  // window is hosted in a GL resource building thread (e.g. texture loader).
  void EndFrame();

  // A context is used to issue draw calls to the framebuffer represented by the parent window.
  ContextPtr context;

  glm::ivec2 Size() const { return size; }

private:
  SDL_Window* window;
  SDL_GLContext glContext;

  glm::ivec2 position;
  glm::ivec2 size;
  bool fullScreen;
  bool visible;

  void MakeWindow(const char* const title);
};

typedef boost::shared_ptr<Window> WindowPtr;

#endif // __WINDOW__
