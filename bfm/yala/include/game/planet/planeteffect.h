#if ! defined(__PLANET_EFFECT__)
#define __PLANET_EFFECT__

#include <core/effect/effect.h>

class PlanetEffect : public Effect
{
public:
  PlanetEffect();
  virtual ~PlanetEffect();

  EffectUniform* SunDirection;
  EffectUniform* Radius;
  EffectUniform* Centre;
  EffectUniform* Width;

private:
  virtual void Initialise();
};

#endif // __PLANET_EFFECT__
