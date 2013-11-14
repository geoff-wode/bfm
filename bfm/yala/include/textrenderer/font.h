#if ! defined(__FONT__)
#define __FONT__

#include <glm/glm.hpp>

#include <device.h>
#include <textrenderer/fonteffect.h>
#include <textures/texture2d.h>
#include <textures/sampler2d.h>
#include <unordered_map>
#include <boost/shared_ptr.hpp>

struct Font;

class FontManager
{
public:
  FontManager();
  ~FontManager();

  bool Initialise(const glm::ivec2& screenResolution);

  // Load the font from the file and set its size in points.
  boost::shared_ptr<Font> LoadFont(const char* const filename);

  void DrawText(boost::shared_ptr<Font> font, Device* const device, const char* const text, const glm::ivec2& position, const glm::vec4& colour);

private:
  typedef std::unordered_map<const char* const, boost::shared_ptr<Font>> FontMap;
  FontMap fonts;

  FontEffect effect;
  glm::ivec2 screenResolution;

  // The vertex data is itself buffered to prevent the GPU choking when the CPU is updating
  // the data for the next frame ...
  static const size_t bufferCount = 3;
  VertexArray geometry[bufferCount];
  size_t geometrySelector;

  boost::shared_ptr<VertexBuffer> vertexBuffer;

  RenderState renderState;
  boost::shared_ptr<Sampler2D> sampler;
};

#endif // __FONT__
