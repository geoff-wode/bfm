#include <SDL.h>
#include <logging.h>
#include <effect.h>
#include <vector>
#include <glfx.h>
#include <boost/foreach.hpp>
#include <glm/ext.hpp>
#include <memory>
#include <boost/make_shared.hpp>

//--------------------------------------------------------------
Effect::Effect()
  : programHandle(0)
{
}

//--------------------------------------------------------------
Effect::~Effect()
{
  glDeleteProgram(programHandle);
}

void Effect::Enable()
{
  glUseProgram(programHandle);
}

//--------------------------------------------------------------
bool Effect::Load(const char* const effectFilename, const char* const programName)
{
  bool loaded = false;

  const int glfx = glfxGenEffect();
  if (glfxParseEffectFromFile(glfx, effectFilename))
  {
    programHandle = glfxCompileProgram(glfx, programName);
    if (-1 != programHandle)
    {
      ParseParams();
      LOG("effect %s::%s compiled - uniform parameters:\n", effectFilename, programName);
      BOOST_FOREACH(auto v, parameters)
      {
        LOG("  %s\n", v.first.c_str());
      }
      Initialise();
      loaded = true;
    }
    else
    {
      const char *log = glfxGetEffectLog(glfx);
      LOG("\n%s::%s\n%s\n", effectFilename, programName, log);
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, programName, log, NULL);
    }
  }
  else
  {
    const char *log = glfxGetEffectLog(glfx);
    LOG("\ncould not parse %s::%s:\n%s\n", effectFilename, programName, log);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, programName, log, NULL);
  }

  glfxDeleteEffect(glfx);

  return loaded;
}

//--------------------------------------------------------------
void Effect::ParseParams()
{
	GLint numUniforms;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORMS, &numUniforms);

	std::vector<GLuint> indices(numUniforms);
	for (GLint i = 0; i < numUniforms; ++i) { indices[i] = i; }

	std::vector<GLint> types(numUniforms);
	glGetActiveUniformsiv(programHandle, numUniforms, indices.data(), GL_UNIFORM_TYPE, types.data());

	std::vector<GLint> blockIndices(numUniforms);
	glGetActiveUniformsiv(programHandle, numUniforms, indices.data(), GL_UNIFORM_BLOCK_INDEX, blockIndices.data());

	for (int i = 0; i < numUniforms; ++i)
	{
		if (-1 == blockIndices[i])
		{
			// not a named uniform block...
			char uniformName[256] = { 0 };
			glGetActiveUniformName(programHandle, i, sizeof(uniformName)-1, NULL, uniformName);
      const GLint location = glGetUniformLocation(programHandle, uniformName);
      if (0 == std::strcmp("WorldMatrix", uniformName))                   
      {
        WorldMatrix.program = programHandle;
        WorldMatrix.location = location;
      }
      else if (0 == std::strcmp("ViewMatrix", uniformName))               
      {
        ViewMatrix.program = programHandle;
        ViewMatrix.location = location;
      }
      else if (0 == std::strcmp("ProjectionMatrix", uniformName))         
      {
        ProjectionMatrix.program = programHandle;
        ProjectionMatrix.location = location;
      }
      else if (0 == std::strcmp("ViewProjectionMatrix", uniformName))     
      {
        ViewProjectionMatrix.program = programHandle;
        ViewProjectionMatrix.location = location;
      }
      else if (0 == std::strcmp("WorldViewMatrix", uniformName))          
      {
        WorldViewMatrix.program = programHandle;
        WorldViewMatrix.location = location;
      }
      else if (0 == std::strcmp("WorldViewProjectionMatrix", uniformName))
      {
        WorldViewProjectionMatrix.program = programHandle;
        WorldViewProjectionMatrix.location = location;
      }

      parameters[uniformName] = location;
		}
	}
}
