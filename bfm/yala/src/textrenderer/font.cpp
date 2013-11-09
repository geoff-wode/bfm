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
#include <textrenderer/font.h>
#include <boost/make_shared.hpp>

#include <ft2build.h>
#include FT_MODULE_H

//---------------------------------------------------------------------
// Yeah FT2, make me jump through a hoop for that error string fish!
#undef __FTERRORS_H__                      
#define FT_ERRORDEF( e, v, s )  { e, s },  
#define FT_ERROR_START_LIST     {          
#define FT_ERROR_END_LIST       { 0, 0 } };
                                             
const struct                               
{                                          
  int          err_code;                   
  const char*  err_msg;                    
} ft_errors[] =           // this is "right" and not just a dangling bit of syntax (ffs!)                                             
#include FT_ERRORS_H
static const size_t FTErrorCount = sizeof(ft_errors) / sizeof(ft_errors[0]);

//----------------------------------------------------------------------

// Screen-space position (xy) and texture coordinate (zw).
struct Vertex
{
  glm::vec4 position;
};

static const VertexAttribute vertexAttribute =
{
  VertexSemantic::Position, GL_FLOAT, 4, offsetof(Vertex, position)
};

// TODO: Currently the buffer is 4 vertices per glyph!
// Improve performance by using a texture atlas to enable multiple character positions to be
// passed to the GPU at once.
// Improve things even more by using a geometry shader.
static const size_t VerticesPerCall = 4;
static const size_t VerticesInBuffer = 2048;
static Vertex vertices[VerticesPerCall];

static const size_t GlyphTextureSlot = 0;

//----------------------------------------------------------------------

static void* Allocate(FT_Memory memory, long size);
static void Deallocate(FT_Memory memory, void* block);
static void* Reallocate(FT_Memory memory, long currentSize, long newSize, void* block);
static const char* const FreeTypeError(int e);

//----------------------------------------------------------------------

Font::Font()
{
}

Font::~Font()
{
  FT_Done_Face(face);
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

  // library shud down:
  FT_Done_Library(freetype);
}

//----------------------------------------------------------------------
bool FontManager::Initialise(const glm::ivec2& screenResolution)
{
  static struct FT_MemoryRec_ memoryManagement =
  {
    NULL,
    Allocate,
    Deallocate,
    Reallocate
  };

  this->screenResolution = screenResolution;

  if (FT_New_Library(&memoryManagement, &freetype))
  {
    LOG("did not initialise FreeType library instance\n");
    return false;
  }
  FT_Add_Default_Modules(freetype);

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
boost::shared_ptr<Font> FontManager::LoadFont(const char* const filename, size_t pointSize)
{
  // Already created a font of this style and size..?
  auto fontPos = fonts.find(filename);
  if (fonts.cend() != fontPos)
  {
    if (fontPos->second->pointSize == pointSize)
    {
      return fontPos->second; // yes.
    }
  }

  // No such font, create it...
  boost::shared_ptr<Font> font(new Font());
  FT_Error err = FT_New_Face(freetype, filename, 0, &font->face);
  if (err)
  {
    LOG("font %s load failed : %s\n", filename, FreeTypeError(err));
    return NULL;
  }
  LOG("font %s loaded\n", filename);
  font->pointSize = pointSize;
  std::strncpy(font->filename, filename, sizeof(font->filename - 1));
  // ...and add it to the cache...
  fonts[font->filename] = font;

  // Set the size:
  static const int dpi = 96;
  pointSize *= 64;
  FT_Set_Char_Size(font->face, pointSize, pointSize, dpi, dpi);

  return font;
}

//----------------------------------------------------------------------
void FontManager::DrawText(boost::shared_ptr<Font> font, Device* const device, const char* const text, const glm::ivec2& position, const glm::vec4& colour)
{
  renderState.vertexArray = &geometry[geometrySelector];
  size_t nextVertex = 0;

  effect.FontColour->Set(colour);

  glm::ivec2 origin = position;
  FT_GlyphSlot slot = font->face->glyph;

  glActiveTexture(GL_TEXTURE0 + GlyphTextureSlot);
  sampler->Bind(GlyphTextureSlot);

  for (const char* c = text; '\0' != *c; ++c)
  {
    if (' ' != *c)
    {
      Glyph* glyph = NULL;

      auto glyphPos = font->glyphs.find(*c);
      if (font->glyphs.cend() != glyphPos)
      {
        glyph = glyphPos->second.get();
      }
      else
      {
        glyph = CreateGlyphTexture(*c, font);
      }

      if (glyph)
      {
        // See http://www.freetype.org/freetype2/docs/tutorial/step2.html for a description of the metrics...
        const float left = origin.x + glyph->left;
        const float right = left + glyph->width;
        const float top = origin.y + glyph->top;
        const float bottom = origin.y + glyph->base;
        const float width = glyph->width;
        const float height = glyph->height;

        // Create a textured quad BUT the FT glyph bitmap is upside down relative to GL:
        vertices[0].position = glm::vec4(left,  bottom, 0, 1);
        vertices[1].position = glm::vec4(left,  top,    0, 0);
        vertices[2].position = glm::vec4(right, bottom, 1, 1);
        vertices[3].position = glm::vec4(right, top,    1, 0);

        renderState.vertexArray->GetVertexBuffer()->Enable();
        renderState.vertexArray->GetVertexBuffer()->SetData(&vertices, VerticesPerCall, nextVertex);
        renderState.vertexArray->GetVertexBuffer()->Disable();

        //renderState.textureUnits[GlyphTextureSlot] = &glyph->texture;

        //glyph->texture.BindToTextureUnit(GlyphTextureSlot);
        glBindTexture(GL_TEXTURE_2D, glyph->texture.texture);

        device->Draw(GL_TRIANGLE_STRIP, nextVertex, VerticesPerCall, renderState);

        nextVertex += VerticesPerCall;
        ASSERT(nextVertex < VerticesInBuffer);
      }
    }

    origin.x += slot->advance.x >> 6;
  }

  // Switch to next buffer (with wraparound)
  geometrySelector = (geometrySelector + 1) % 3;
}

//----------------------------------------------------------------------
Glyph* FontManager::CreateGlyphTexture(char c, boost::shared_ptr<Font> font)
{
  if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
  {
    LOG("did not render glyph for '%c'\n", c);
    return NULL;
  }

  FT_GlyphSlot slot = font->face->glyph;
  boost::shared_ptr<Glyph> glyph(new Glyph());

  glyph->left = float(slot->metrics.horiBearingX >> 6);
  glyph->top = float(slot->metrics.horiBearingY >> 6);
  glyph->base = glyph->top - float(slot->metrics.height >> 6);
  glyph->width = float(slot->metrics.width >> 6);
  glyph->height = float(slot->metrics.height >> 6);

  // Ensure upload of data to the texture is byte-aligned (and restore the current alignemt on exit)
  GLint alignment;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glyph->texture.AssignSampler(sampler);
  glyph->texture.Load(slot->bitmap.buffer, slot->bitmap.pitch, slot->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, GL_ALPHA);

  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

  font->glyphs[c] = glyph;

  return font->glyphs[c].get();
}

//----------------------------------------------------------------------

static void* Allocate(FT_Memory memory, long size)
{
  return malloc(size);
}

static void Deallocate(FT_Memory memory, void* block)
{
  free(block);
}

static void* Reallocate(FT_Memory memory, long currentSize, long newSize, void* block)
{
  if (currentSize != newSize)
  {
    block = realloc(block, newSize);
  }
  return block;
}

static const char* const FreeTypeError(int e)
{
  for (int i = 0; i < FTErrorCount; ++i)
  {
    if (ft_errors[i].err_code == e) { return ft_errors[i].err_msg; }
  }
  return "unknown";
}
