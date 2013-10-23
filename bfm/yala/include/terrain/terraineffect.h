#if ! defined(__TERRAIN_EFFECT__)
#define __TERRAIN_EFFECT__

#include <effect.h>
#include <effectuniform.h>

// Class representing an effect which renders terrain "relative-to-eye" space,
// allowing for massive objects and view distances.
class TerrainEffect : public Effect
{
public:
  TerrainEffect() { }
  virtual ~TerrainEffect() { }

  EffectUniform* MaxHeight;
  EffectUniform* TileWidth;
  EffectUniform* TileCentre;

private:
  virtual void Initialise()
  {
    MaxHeight = &parameters["MaxHeight"];
    TileWidth = &parameters["TileWidth"];
    TileCentre = &parameters["TileCentre"];

    Effect::Initialise();
  }
};

#endif // __TERRAIN_EFFECT__
