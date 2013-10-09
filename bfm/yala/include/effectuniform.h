#if ! defined(__EFFECT_UNIFORM__)
#define __EFFECT_UNIFORM__

#include <glm/glm.hpp>
#include <gl_loader/gl_loader.h>

//---------------------------------------------------------------------------------------

class EffectUniform
{
public:
  EffectUniform()
    : location(-1), modified(false)
  {
  }

  void Set(float value);
  void Set(int value);
  void Set(unsigned int value);
  void Set(const glm::vec2& value);
  void Set(const glm::vec3& value);
  void Set(const glm::vec4& value);
  void Set(const glm::mat2& value);
  void Set(const glm::mat3& value);
  void Set(const glm::mat4& value);

private:
  GLint location;
  GLenum type;
  bool modified;
  double cache[16];

  friend class Effect;
};

#endif // __EFFECT_UNIFORM__
