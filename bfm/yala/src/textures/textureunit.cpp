#include <device.h>
#include <textures/textureunit.h>
#include <gl_loader/gl_loader.h>

//-------------------------------------------------------

TextureUnit::TextureUnit(size_t slot)
  : slot(slot)
{
}

TextureUnit::~TextureUnit()
{
}

//-------------------------------------------------------

void TextureUnit::Enable()
{
  glActiveTexture(GL_TEXTURE0 + slot);

  if (texture) { texture->Bind(); }
  if (sampler) { sampler->Bind(slot); }

  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);
}

//-------------------------------------------------------

void TextureUnit::Disable()
{
  glActiveTexture(GL_TEXTURE0 + slot);

  if (texture) { texture->Unbind(); }
  if (sampler) { sampler->Unbind(slot); }

  glActiveTexture(GL_TEXTURE0 + RenderState::MaxTextures);
}
