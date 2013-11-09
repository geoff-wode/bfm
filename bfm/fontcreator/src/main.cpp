/*
 Reads a TrueType font definition file and outputs a texture atlas for the 7 bit printable
 ASCII characters.
 A binary file containing the meta data for each character is also created.
 The following is stored for the overall font:

      number of glyphs
      array of glyph data

 The following is stored for each glyph:

      glyph code
      texture X coordinate of glyph's bottom-left corner
      width & height in texels
      horizontal offset from origin to left edge _in_pixels_
      vertical offset from origin to bottom edge _in_pixels_
      horizontal "pen advance" to start of next glyph _in_pixels_
 */

#include <SDL.h>
#include <SOIL.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H

//----------------------------------------------------------------------

struct GlyphData
{
  int left;     // distance from origin to left edge of glyph when rendered
  int top;      // distance from origin to top edge of glyph when rendered
  int advance;  // horizontal distance to start of next glyph
  int width;
  int height;
  int textureLeftEdge;
  std::vector<unsigned char> bitmap;
};

struct GlyphMetaData
{
  size_t code;
  glm::i8vec2 bearing;
  size_t penAdvance;
  glm::half textureLeftEdge;
  glm::hvec2 textureSize;
};

struct FontMetaData
{
  size_t glyphCount;
  std::vector<GlyphMetaData> glyphs;
};

//----------------------------------------------------------------------

static FT_Library freetype;
static FT_Face fontFace;

static const size_t GlyphCount = '~' - ' ';
static GlyphData glyphs[GlyphCount];
static char textureFilename[256];
static char metadataFilename[256];

static size_t pointSize;
static const char* fontFilename;
static const char* atlasName;

static const char* const imageFileExtension[] =
{
  "tga",
	"bmp",
	"dds"
};
static const int imageType = SOIL_SAVE_TYPE_DDS;

//----------------------------------------------------------------------

static void AtExit(void)
{
  FT_Done_Face(fontFace);
  FT_Done_Library(freetype);
}

//----------------------------------------------------------------------

static void SaveTexture(const glm::ivec2& textureSize);
static void SaveMetaData(const glm::ivec2& textureSize);

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  fontFilename = argv[1];
  pointSize = std::atoi(argv[2]);
  atlasName = argv[3];

  if (!fontFilename || !atlasName)
  {
    std::printf("invalid filename\n");
    std::exit(EXIT_FAILURE);
  }

  std::sprintf(textureFilename, "%s@%d.%s", atlasName, pointSize, imageFileExtension[imageType]);
  std::sprintf(metadataFilename, "%s@%d.dat", atlasName, pointSize);

  if (FT_Init_FreeType(&freetype))
  {
    std::printf("failed to initialise FT\n");
    std::exit(EXIT_FAILURE);
  }
  FT_Add_Default_Modules(freetype);

  if (FT_New_Face(freetype, fontFilename, 0, &fontFace))
  {
    std::printf("failed to load font\n");
    exit(EXIT_FAILURE);
  }

  // Set the size:
  static const int dpi = 96;
  FT_Set_Char_Size(fontFace, pointSize * 64, pointSize * 64, dpi, dpi);
  
  glm::ivec2 textureSize(0);

  // Collect meta-data...
  size_t horizontalOffset = 0;
  for (size_t i = 0; i < GlyphCount; ++i)
  {
    const char c = i + '!';
    FT_Load_Char(fontFace, c, FT_LOAD_RENDER);
    const FT_GlyphSlot slot = fontFace->glyph;

    glyphs[i].advance = slot->metrics.horiAdvance >> 6;
    glyphs[i].left = slot->metrics.horiBearingX >> 6;
    glyphs[i].top = slot->metrics.horiBearingY >> 6;
    glyphs[i].width = slot->bitmap.width;
    glyphs[i].height = slot->bitmap.rows;
    glyphs[i].textureLeftEdge = textureSize.x;

    glyphs[i].bitmap.resize(glyphs[i].width * glyphs[i].height);
    std::memcpy(glyphs[i].bitmap.data(), slot->bitmap.buffer, glyphs[i].bitmap.size());

    textureSize.x += glyphs[i].width;
    textureSize.y = glm::max(textureSize.y, glyphs[i].height);
  }

  SaveTexture(textureSize);
  SaveMetaData(textureSize);

  return 0;
}

//----------------------------------------------------------------------

static void SaveTexture(const glm::ivec2& textureSize)
{
  // Build a single texture with each glyph bitmap stored tightly packed into a horizontal strip...
  std::vector<unsigned char> texture(textureSize.x * textureSize.y);

  for (size_t i = 0; i < GlyphCount; ++i)
  {
    unsigned char* textureRow = texture.data() + glyphs[i].textureLeftEdge;
    unsigned char* bitmapRow = glyphs[i].bitmap.data();
    const size_t pitch = glyphs[i].width;

    for (int y = 0; y < glyphs[i].height; ++y)
    {
      std::memcpy(textureRow, bitmapRow, pitch);
      textureRow += textureSize.x;
      bitmapRow += pitch;
    }
  }

  // Write out the texture...
  FILE* textureFile = fopen(textureFilename, "wb");
  SOIL_save_image(textureFilename, imageType, textureSize.x, textureSize.y, 1, texture.data());
  std::fclose(textureFile);
}

//----------------------------------------------------------------------

static void SaveMetaData(const glm::ivec2& textureSize)
{
  FontMetaData fontMetaData;

  fontMetaData.glyphCount = GlyphCount;
  fontMetaData.glyphs.resize(GlyphCount);

  // Determine how "big" one texel is in the horizontal. This will be used to compute
  // the texture coordinates of each individual glyph.
  const glm::hvec2 oneTexel = glm::hvec2(1.0f / glm::vec2(textureSize));

  for (size_t i = 0; i < GlyphCount; ++i)
  {
    GlyphMetaData& glyphData = fontMetaData.glyphs[i];
    glyphData.code = i + '!';
    glyphData.penAdvance = glyphs[i].advance;
    glyphData.bearing.x = glyphs[i].left;
    glyphData.bearing.y = glyphs[i].top - glyphs[i].height;
    glyphData.textureLeftEdge = oneTexel.x * glm::half(glyphs[i].textureLeftEdge);
    glyphData.textureSize = oneTexel * glm::hvec2(glyphs[i].width, glyphs[i].height);
  }

  FILE* metadataFile = fopen(metadataFilename, "wb");
  fwrite(&fontMetaData.glyphCount, sizeof(fontMetaData.glyphCount), 1, metadataFile);
  for (size_t i = 0; i < fontMetaData.glyphs.size(); ++i)
  {
    GlyphMetaData& glyphData = fontMetaData.glyphs[i];

#define SAVE_GLYPH_DATA(file, member) fwrite(&member, sizeof(member), 1, file);

    SAVE_GLYPH_DATA(metadataFile, glyphData.code);
    SAVE_GLYPH_DATA(metadataFile, glyphData.bearing.x);
    SAVE_GLYPH_DATA(metadataFile, glyphData.bearing.y);
    SAVE_GLYPH_DATA(metadataFile, glyphData.penAdvance);
    SAVE_GLYPH_DATA(metadataFile, glyphData.textureLeftEdge);
    SAVE_GLYPH_DATA(metadataFile, glyphData.textureSize.x);
    SAVE_GLYPH_DATA(metadataFile, glyphData.textureSize.y);
  }
  fclose(metadataFile);
}
