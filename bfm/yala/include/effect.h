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

  // The following uniform parameters are automatically available to any effect which
  // uses them.
  FloatMat4EffectUniform WorldMatrix;
  FloatMat4EffectUniform ViewMatrix;
  FloatMat4EffectUniform ProjectionMatrix;
  FloatMat4EffectUniform ViewProjectionMatrix;
  FloatMat4EffectUniform WorldViewMatrix;
  FloatMat4EffectUniform WorldViewProjectionMatrix;

protected:
  typedef std::map<std::string, GLuint> ParameterList;
  ParameterList parameters;

  GLuint programHandle;

private:
  void ParseParams();
  virtual void Initialise() { }
};

#endif // __EFFECT__
