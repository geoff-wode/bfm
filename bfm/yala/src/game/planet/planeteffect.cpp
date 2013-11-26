#include <game/planet/planeteffect.h>

//-------------------------------------------------------------------------------------------

PlanetEffect::PlanetEffect()
{
}

//-------------------------------------------------------------------------------------------

PlanetEffect::~PlanetEffect()
{
}

//-------------------------------------------------------------------------------------------

void PlanetEffect::Initialise()
{
  SunDirection= &parameters["SunDirection"];
  Radius = &parameters["Radius"];
  Centre = &parameters["Centre"];
  Width = &parameters["Width"];

  Effect::Initialise();
}
