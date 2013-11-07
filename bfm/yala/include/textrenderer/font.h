#if ! defined(__FONT__)
#define __FONT__

#include <unordered_map>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <fileio.h>
#include <device.h>
#include <textrenderer/fonteffect.h>
#include <textures/texture2d.h>
#include <textures/sampler2d.h>

struct Glyph
{
  // See http://www.freetype.org/freetype2/docs/tutorial/step2.html for extra details.

  float left;   // offset from origin to start of bitmap (bearingX)
  float top;    // offset from origin to top of bitmap (bearingY)
  float base;   // offset from origin to base of bitmap (top - height)
  float width;  // of bitmap
  float height; // of bitmap
  Texture2D texture;
};

struct Font
{
  Font();
  ~Font();

  FT_Face face;
  size_t pointSize;
  char filename[MAX_PATH];
  std::unordered_map<char, boost::shared_ptr<Glyph>> glyphs;
};


class FontManager
{
public:
  FontManager();
  ~FontManager();

  bool Initialise(const glm::ivec2& screenResolution);

  // Load the font from the file and set its size in points.
  boost::shared_ptr<Font> LoadFont(const char* const filename, size_t pointSize);

  void DrawText(boost::shared_ptr<Font> font, Device* const device, const char* const text, const glm::ivec2& position, const glm::vec4& colour);

private:
  typedef std::unordered_map<const char* const, boost::shared_ptr<Font>> FontMap;
  FontMap fonts;

  FT_Library freetype;

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

  Glyph* CreateGlyphTexture(char c, boost::shared_ptr<Font> font);
};

#endif // __FONT__
