#if ! defined(__UNIFORM_BUFFER__)
#define __UNIFORM_BUFFER__

#include <cstddef>
#include <gl_loader/gl_loader.h>

class UniformBuffer
{
public:
  UniformBuffer(size_t size);
  ~UniformBuffer();

  void BindTo(GLuint bindingPoint);
  void BindTo(GLuint bindingPoint, size_t size, size_t offset);

  void Enable() { glBindBuffer(GL_UNIFORM_BUFFER, buffer); }
  void Disable() { glBindBuffer(GL_UNIFORM_BUFFER, 0); }

  void SetData(const void* const data, size_t size, size_t offset = 0);

private:
  const size_t size;
  GLuint buffer;
};


#endif // __UNIFORM_BUFFER__
