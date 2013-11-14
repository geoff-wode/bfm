/*
 TODO: At the moment, each character is rendered seperately using its own
       unique texture. This means that each character must be rendered
       seperately via its own draw call.
       This can be drastically improved by creating a texture atlas of the
       font's character set and passing the texture coordinates for each
       character as well as the position in a vertex buffer.
*/

#include <vector>
#include <cstring>
#include <logging.h>
#include <fileio.h>
#include <SOIL.h>
#include <textrenderer/font.h>
#include <boost/make_shared.hpp>
#include <glm/ext.hpp>

//----------------------------------------------------------------------

// Screen-space position (xy) and texture coordinate (zw).
struct Vertex
{
  glm::vec4 position;
};

struct GlyphMetaData
{
  size_t code;
  glm::i8vec2 bearing;
  glm::i8 penAdvance;
  glm::i8vec2 size;
  glm::half textureLeftEdge;
  glm::hvec2 textureSize;
};

struct Glyph
{
  glm::ivec2 bearing;
  glm::ivec2 size;
  size_t penAdvance;
  float textureLeftEdge;
  glm::vec2 textureSize;
};

struct Font
{
  Font();
  ~Font();

  Texture2D texture;

  size_t spaceAdvance;
  std::vector<Glyph> glyphs;
  std::unordered_map<size_t, Glyph*> glyphMap;
};

static const VertexAttribute vertexAttribute =
{
  VertexSemantic::Position, GL_FLOAT, 4, offsetof(Vertex, position)
};

// TODO: Currently the buffer is 4 vertices per glyph!
// Improve performance by using a texture atlas to enable multiple character positions to be
// passed to the GPU at once.
// Improve things even more by using a geometry shader.
static const size_t VerticesPerGlyph = 4;
static const size_t VerticesInBuffer = 2048;
static Vertex vertices[VerticesInBuffer * VerticesPerGlyph];

static const size_t GlyphTextureSlot = 0;

//----------------------------------------------------------------------

static boost::shared_ptr<Font> CreateFont(const char* const filename);

//----------------------------------------------------------------------

Font::Font()
{
}

Font::~Font()
{
}

//----------------------------------------------------------------------

FontManager::FontManager()
  : geometrySelector(0)
{
  renderState.effect = &effect;
  renderState.drawState.culling.enabled = false;
  renderState.drawState.depthTest.enabled = false;
  renderState.drawState.depthBufferMask = false;
  renderState.drawState.blending.enabled = true;
  renderState.drawState.blending.rgb.source = GL_SRC_ALPHA;
  renderState.drawState.blending.alpha.source = GL_SRC_ALPHA;
  renderState.drawState.blending.rgb.destination = GL_ONE_MINUS_SRC_ALPHA;
  renderState.drawState.blending.alpha.destination = GL_ONE_MINUS_SRC_ALPHA;
}

//----------------------------------------------------------------------
FontManager::~FontManager()
{
  // make sure the cached fonts are individually tidied away _before_ the library is shut down:
  fonts.clear();
}

//----------------------------------------------------------------------
bool FontManager::Initialise(const glm::ivec2& screenResolution)
{
  this->screenResolution = screenResolution;

  if (!effect.Load("assets\\effects\\fonteffect.glsl", "RenderFont"))
  {
    LOG("did not load font rendering effect\n");
    return false;
  }
  // Since the screen resolution does not change at runtime, set this now...
  effect.ScreenSize->Set(glm::vec2(screenResolution));
  effect.GlyphTexture->Set(GlyphTextureSlot);

  sampler = boost::make_shared<Sampler2D>();
  sampler->SetMagFilter(GL_LINEAR);
  sampler->SetMinFilter(GL_LINEAR);
  sampler->SetWrapS(GL_CLAMP_TO_EDGE);
  sampler->SetWrapT(GL_CLAMP_TO_EDGE);

  VertexLayout vertexLayout;
  vertexLayout.AddAttribute(vertexAttribute);

  // Initialise vertex buffers for dynamic update...
  for (size_t i = 0; i < bufferCount; ++i)
  {
    boost::shared_ptr<VertexBuffer> vertexBuffer(new VertexBuffer());
    vertexBuffer->Initialise(vertexLayout, VerticesInBuffer, GL_DYNAMIC_DRAW);
    geometry[i].Initialise(vertexBuffer);
  }

  return true;
}

//----------------------------------------------------------------------
boost::shared_ptr<Font> FontManager::LoadFont(const char* const filename)
{
  // Already created a font of this style and size..?
  auto fontPos = fonts.find(filename);
  if (fonts.cend() != fontPos)
  {
    return fontPos->second; // yes.
  }

  // No such font, create it...
  boost::shared_ptr<Font> font = CreateFont(filename);

  return font;
}

//----------------------------------------------------------------------
void FontManager::DrawText(boost::shared_ptr<Font> font, Device* const device, const char* const text, const glm::ivec2& position, const glm::vec4& colour)
{
  ASSERT(strlen(text) <= VerticesInBuffer);

  renderState.vertexArray = &geometry[geometrySelector];

  effect.FontColour->Set(colour);

  glm::ivec2 origin = position;

  glActiveTexture(GL_TEXTURE0 + GlyphTextureSlot);
  sampler->Bind(GlyphTextureSlot);
  glBindTexture(GL_TEXTURE_2D, font->texture.texture);

  size_t nextVertex;
  for (nextVertex = 0; text[nextVertex] != '\0'; ++nextVertex)
  {
    const char c = text[nextVertex];

    if (' ' != c)
    {
      Glyph* glyph = NULL;

      auto glyphPos = font->glyphMap.find(c);
      if (font->glyphMap.cend() != glyphPos)
      {
        glyph = glyphPos->second;
      }

      if (glyph)
      {
        const int width = glyph->size.x;
        const int height = glyph->size.y;
        const int left = origin.x + glyph->bearing.x;
        const int right = left + width;
        const int top = origin.y + glyph->bearing.y;
        const int bottom = top - height;

        // Create a textured quad...
        Vertex* v = &vertices[VerticesPerGlyph * nextVertex];
        v[0].position = glm::vec4(left,  bottom, glyph->textureLeftEdge, glyph->textureSize.y);
        v[1].position = glm::vec4(left,  top,    glyph->textureLeftEdge, 0);
        v[2].position = glm::vec4(right, bottom, glyph->textureLeftEdge + glyph->textureSize.x, 0);
        v[3].position = glm::vec4(right, top,    glyph->textureLeftEdge + glyph->textureSize.x, glyph->textureSize.y);
      }
      origin.x += glyph->penAdvance;
    }
    else
    {
      origin.x += font->spaceAdvance;
    }

    renderState.vertexArray->GetVertexBuffer()->Enable();
    renderState.vertexArray->GetVertexBuffer()->SetData(&vertices, VerticesPerGlyph * nextVertex);
    renderState.vertexArray->GetVertexBuffer()->Disable();

    device->Draw(GL_TRIANGLE_STRIP, nextVertex, VerticesPerGlyph, renderState);
  }

  // Switch to next buffer (with wraparound)
  geometrySelector = (geometrySelector + 1) % 3;
}

//----------------------------------------------------------------------

static boost::shared_ptr<Font> CreateFont(const char* const filename)
{
  boost::shared_ptr<Font> font(new Font());

  std::vector<char> textureName(strlen(filename) + sizeof(".dds"));
  sprintf(textureName.data(), "%s.dds", filename);
  font->texture.Load(textureName.data(), GL_ALPHA, GL_UNSIGNED_BYTE, GL_ALPHA);
 
  std::vector<char> metadataFilename(strlen(filename) + sizeof(".dat"));
  sprintf(metadataFilename.data(), "%s.dat", filename);

  FILE* file(fopen(metadataFilename.data(), "rb"));

  size_t glyphCount;
  fread(&glyphCount, sizeof(glyphCount), 1, file);
  font->glyphs.resize(glyphCount);
  fread(&font->spaceAdvance, sizeof(font->spaceAdvance), 1, file);

  for (size_t i = 0; i < glyphCount; ++i)
  {
    GlyphMetaData metadata;

#define LOAD_GLYPH_DATA(file, member) fread(&member, sizeof(member), 1, file);
    LOAD_GLYPH_DATA(file, metadata.code);
    LOAD_GLYPH_DATA(file, metadata.bearing.x);
    LOAD_GLYPH_DATA(file, metadata.bearing.y);
    LOAD_GLYPH_DATA(file, metadata.size.y);
    LOAD_GLYPH_DATA(file, metadata.size.y);
    LOAD_GLYPH_DATA(file, metadata.penAdvance);
    LOAD_GLYPH_DATA(file, metadata.textureLeftEdge);
    LOAD_GLYPH_DATA(file, metadata.textureSize.x);
    LOAD_GLYPH_DATA(file, metadata.textureSize.y);

    font->glyphs[i].bearing = metadata.bearing;
    font->glyphs[i].penAdvance = metadata.penAdvance;
    font->glyphs[i].size = metadata.size;
    font->glyphs[i].textureLeftEdge = metadata.textureLeftEdge;
    font->glyphs[i].textureSize = metadata.textureSize;

    font->glyphMap[metadata.code] = &font->glyphs[i];
  }

  fclose(file);

  return font;
}
