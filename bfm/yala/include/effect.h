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

protected:
  typedef std::map<std::string, EffectUniform> ParameterList;
  ParameterList parameters;

  GLuint programHandle;

private:
  void ParseParams();
  virtual void Initialise() { }
};

#endif // __EFFECT__
