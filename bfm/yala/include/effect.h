#if ! defined(__EFFECT__)
#define __EFFECT__

#include <effectuniform.h>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <gl_loader/gl_loader.h>
#include <boost/shared_ptr.hpp>

class Effect
{
public:
  Effect();
  virtual ~Effect();

  bool Load(const char* const effectFilename, const char* const programName);

  // Call this to make the effect "active".
  void Enable();

  // Update any changes to the effect's parameters which have happened since the last time
  // Apply was called.
  // Called automatically by Device::Draw to ensure the GPU remains sync'd with the CPU.
  void Appply();

  EffectUniform* WorldMatrix;
  EffectUniform* ViewMatrix;
  EffectUniform* ProjectionMatrix;
  EffectUniform* ViewProjectionMatrix;
  EffectUniform* WorldViewProjectionMatrix;

  EffectUniform* CameraPosition;

  // Values to configure the computation of logarithmic rather than linear depth.
  // See http://www.gamedev.net/blog/73/entry-2006307-tip-of-the-day-logarithmic-zbuffer-artifacts-fix
  // but basically:
  //
  //          | log((C * zClip) + offset) |
  //  depth = | ------------------------- |
  //          |  log((C * far) + offset)  |
  //
  // where,
  // - C is a constant controlling depth precision (smaller increases precision at
  //    distance at the expense of loosing precision close-in.
  // - zClip is the clip-space (ie. post-projection transform) z coordinate
  // - offset controls how close the near clip plane can be to the camera (bigger == less near clipping)
  //    (e.g. 2.0 gives a clip plane of a few centimeters)
  // - far is the distance from the camera to the far clip plane (in world units).

  // Since the divisor is constant for a whole render frame, it is precomputed on the CPU.
  EffectUniform* LogDepthConstant;
  EffectUniform* LogDepthOffset;
  EffectUniform* LogDepthDivisor;

protected:
  virtual void Initialise();

  typedef std::map<std::string, EffectUniform> ParameterList;
  ParameterList parameters;

  GLuint programHandle;

private:
  void ParseParams();
};

#endif // __EFFECT__
