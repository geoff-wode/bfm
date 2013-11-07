// Effect file for rendering a FreeType font glyph.
// The effect uses a geometry shader to convert the pixel position of the glyph
// into a textured quad.
// The input position is the left-hand edge of the glyph's base line.

#include "semantics.glsl"

uniform vec2 ScreenSize;  // width (x) and height (y) of screen in pixels
uniform vec4 FontColour;
uniform sampler2D GlyphTexture;

//---------------------------------------------------------

interface VStoFS
{
  vec2 textureCoord;
};

//---------------------------------------------------------

// Simple(-ish) vertex shader that takes a screen XY position in pixels and
// converts it to normalised device coordinates.
shader VertexShader(
    in vec4 Position : SHADER_SEMANTIC_POSITION,
    out VStoFS outputs
  )
{
  vec2 pixelPos = Position.xy;
  vec2 ndcPos = (2 * (pixelPos / ScreenSize)) - 1;
  gl_Position = vec4(ndcPos, 0, 1);

  outputs.textureCoord = Position.zw;
}

//---------------------------------------------------------

shader FragmentShader(in VStoFS inputs, out vec4 colour)
{
  // Multiply all the input colour's components by the alpha value of the glyph.
  // If the resulting colour is completely transparent, don't bother drawing it.
  colour = FontColour * vec4(1,1,1,texture2D(GlyphTexture, inputs.textureCoord).a);
  if (0.0 == colour.a)
  {
    discard;
  }
}

program RenderFont
{
  vs(420) = VertexShader();
  fs(420) = FragmentShader();
};
