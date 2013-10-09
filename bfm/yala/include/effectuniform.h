#if ! defined(__EFFECT_UNIFORM__)
#define __EFFECT_UNIFORM__

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <memory>
#include <gl_loader/gl_loader.h>

//---------------------------------------------------------------------------------------
template <typename T>
class EffectUniform
{
public:
  EffectUniform()
    : program(-1), location(-1)
  {
  }

  void Set(const T& value)
  {
    if ((-1 != location) && (0 != std::memcmp(&cache, &value, sizeof(T))))
    {
      std::memcpy(&cache, &value, sizeof(T));
      SetData(program, location, cache);
    }
  }

private:
  virtual void SetData(GLuint program, GLint location, const T& data) = 0;
  GLuint program;
  GLint location;
  T cache;

  friend class Effect;
};

//---------------------------------------------------------------------------------------
class FloatEffectUniform : public EffectUniform<float>
{
private:
  virtual void SetData(GLuint program, GLint location, const float& data)
  {
    glProgramUniform1fv(program, location, 1, &data);
  }
};

//---------------------------------------------------------------------------------------
class FloatVec2EffectUniform : public EffectUniform<glm::vec2>
{
public:
private:
  virtual void SetData(GLuint program, GLint location, const glm::vec2& data)
  {
    glProgramUniform2fv(program, location, 1, glm::value_ptr(data));
  }
};

//---------------------------------------------------------------------------------------
class FloatVec3EffectUniform : public EffectUniform<glm::vec3>
{
public:
private:
  virtual void SetData(GLuint program, GLint location, const glm::vec3& data)
  {
    glProgramUniform3fv(program, location, 1, glm::value_ptr(data));
  }
};

//---------------------------------------------------------------------------------------
class FloatVec4EffectUniform : public EffectUniform<glm::vec4>
{
public:
private:
  virtual void SetData(GLuint program, GLint location, const glm::vec4& data)
  {
    glProgramUniform4fv(program, location, 1, glm::value_ptr(data));
  }
};

//---------------------------------------------------------------------------------------
class FloatMat2EffectUniform : public EffectUniform<glm::mat2>
{
public:
private:
  virtual void SetData(GLuint program, GLint location, const glm::mat2& data)
  {
    glProgramUniformMatrix2fv(program, location, 1, GL_FALSE, glm::value_ptr(data));
  }
};

//---------------------------------------------------------------------------------------
class FloatMat3EffectUniform : public EffectUniform<glm::mat3>
{
public:
private:
  virtual void SetData(GLuint program, GLint location, const glm::mat3& data)
  {
    glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, glm::value_ptr(data));
  }
};

//---------------------------------------------------------------------------------------
class FloatMat4EffectUniform : public EffectUniform<glm::mat4>
{
public:
private:
  virtual void SetData(GLuint program, GLint location, const glm::mat4& data)
  {
    glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, glm::value_ptr(data));
  }
};

#endif // __EFFECT_UNIFORM__
