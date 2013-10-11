#if ! defined(__RTE_RENDER_GEOMETRY_EFFECT__)
#define __RTE_RENDER_GEOMETRY_EFFECT__

#include <effect.h>
#include <effectuniform.h>

class RTERenderGeometryEffect : public Effect
{
public:
  RTERenderGeometryEffect();
  virtual ~RTERenderGeometryEffect();

  EffectUniform* RTEWorldViewProjectionMatrix;
  EffectUniform* CameraPositionLow;
  EffectUniform* CameraPositionHigh;
  EffectUniform* FarClipPlaneDistance;
  EffectUniform* LogDepthConstant;

private:
  virtual void Initialise();
};

#endif // __RTE_RENDER_GEOMETRY_EFFECT__
