#include <glm/ext.hpp>
#include <memory>
#include <gl_loader/gl_loader.h>
#include <core/effect/effectuniform.h>

//---------------------------------------------------------------------------------------

static bool SetValue(const void* const value, size_t size, void* const cache);

//---------------------------------------------------------------------------------------

void EffectUniform::Set(float value)            { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(double value)           { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(int value)              { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(unsigned int value)     { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::vec2& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::vec3& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::vec4& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::dvec2& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::dvec3& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::dvec4& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::mat2& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::mat3& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::mat4& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::dmat2& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::dmat3& value) { modified |= SetValue(&value, sizeof(value), cache); }
void EffectUniform::Set(const glm::dmat4& value) { modified |= SetValue(&value, sizeof(value), cache); }

//---------------------------------------------------------------------------------------
static bool SetValue(const void* const value, size_t size, void* const cache)
{
  bool cached = false;
  
  if (0 != std::memcmp(cache, value, size))
  {
    std::memcpy(cache, value, size);
    cached = true;
  }
 
  return cached;
}
