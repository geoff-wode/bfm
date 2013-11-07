#if ! defined(__FONT_EFFECT__)
#define __FONT_EFFECT__

#include <effect.h>
#include <effectuniform.h>

class FontEffect :  public Effect
{
public:
  FontEffect() { }
  virtual ~FontEffect() { }

  EffectUniform* ScreenSize;
  EffectUniform* GlyphTexture;
  EffectUniform* FontColour;

private:
  virtual void Initialise()
  {
    ScreenSize = &parameters["ScreenSize"];
    GlyphTexture = &parameters["GlyphTexture"];
    FontColour = &parameters["FontColour"];

    Effect::Initialise();
  }
};

#endif // __FONT_EFFECT__
