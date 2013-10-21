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

  EffectUniform* CameraPosition;
  EffectUniform* Radius;
  EffectUniform* Centre;
  EffectUniform* Width;

private:
  virtual void Initialise()
  {
    CameraPosition = &parameters["CameraPosition"];
    Radius = &parameters["Radius"];
    Centre = &parameters["Centre"];
    Width = &parameters["Width"];
    Effect::Initialise();
  }
};

#endif // __TERRAIN_EFFECT__
