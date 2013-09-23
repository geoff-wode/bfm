#include <gl_loader/gl_loader.h>

bool glInitLibrary()
{
  bool initialised = false;

#if defined(POINTER_C_GENERATED_HEADER_OPENGL_H)
  ogl_LoadFunctions();
#endif


#if defined(__GLEW_H__)
  GLenum result = glewInit();
  if (GLEW_OK == result)
  {
    initialised = true;
  }
#endif

  return initialised;
}
