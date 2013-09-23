// Avoids the need for code files to know the precise name of the OpenGL
// function loader.

#if ! defined(GL_LOADER)
#define GL_LOADER

//#include "gl_core_4.2.h"

#include <GL/glew.h>


bool glInitLibrary();

#endif // GL_LOADER
