#include <SDL.h>
#include <cstring>
#include <vector>
#include <memory>
#include <glfx.h>
#include <glm/ext.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <gl_loader/gl_loader.h>

#include <core/logging.h>
#include <core/effect/effect.h>

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
void Effect::Apply()
{
  BOOST_FOREACH(auto param, parameters)
  {
    if (param.second.modified)
    {
      switch (param.second.type)
      {
      case GL_INT:          glUniform1iv(param.second.location, 1, (int*)param.second.cache); break;
      case GL_UNSIGNED_INT: glUniform1uiv(param.second.location, 1, (unsigned int*)param.second.cache); break;

      case GL_FLOAT:        glUniform1fv(param.second.location, 1, (float*)param.second.cache); break;
      case GL_FLOAT_VEC2:   glUniform2fv(param.second.location, 1, (float*)param.second.cache); break;
      case GL_FLOAT_VEC3:   glUniform3fv(param.second.location, 1, (float*)param.second.cache); break;
      case GL_FLOAT_VEC4:   glUniform4fv(param.second.location, 1, (float*)param.second.cache); break;

      case GL_DOUBLE:       glUniform1dv(param.second.location, 1, param.second.cache); break;
      case GL_DOUBLE_VEC2:  glUniform2dv(param.second.location, 1, param.second.cache); break;
      case GL_DOUBLE_VEC3:  glUniform3dv(param.second.location, 1, param.second.cache); break;
      case GL_DOUBLE_VEC4:  glUniform4dv(param.second.location, 1, param.second.cache); break;

      case GL_FLOAT_MAT2:   glUniformMatrix2fv(param.second.location, 1, GL_FALSE, (float*)param.second.cache); break;
      case GL_FLOAT_MAT3:   glUniformMatrix3fv(param.second.location, 1, GL_FALSE, (float*)param.second.cache); break;
      case GL_FLOAT_MAT4:   glUniformMatrix4fv(param.second.location, 1, GL_FALSE, (float*)param.second.cache); break;

      case GL_DOUBLE_MAT2:  glUniformMatrix2dv(param.second.location, 1, GL_FALSE, param.second.cache); break;
      case GL_DOUBLE_MAT3:  glUniformMatrix3dv(param.second.location, 1, GL_FALSE, param.second.cache); break;
      case GL_DOUBLE_MAT4:  glUniformMatrix4dv(param.second.location, 1, GL_FALSE, param.second.cache); break;

      case GL_SAMPLER_1D:   glUniform1iv(param.second.location, 1, (int*)param.second.cache); break;
      case GL_SAMPLER_2D:   glUniform1iv(param.second.location, 1, (int*)param.second.cache); break;
      case GL_SAMPLER_3D:   glUniform1iv(param.second.location, 1, (int*)param.second.cache); break;
        
      default: break;
      }
      param.second.modified = false;
    }
  }
}

//--------------------------------------------------------------
void Effect::Initialise()
{
  WorldMatrix = &parameters["WorldMatrix"];
  ViewMatrix = &parameters["ViewMatrix"];
  ProjectionMatrix = &parameters["ProjectionMatrix"];
  ViewProjectionMatrix = &parameters["ViewProjectionMatrix"];
  WorldViewProjectionMatrix = &parameters["WorldViewProjectionMatrix"];
  CameraPosition = &parameters["CameraPosition"];
  LogDepthDivisor = &parameters["LogDepthDivisor"];
  LogDepthConstant = &parameters["LogDepthConstant"];
  LogDepthOffset = &parameters["LogDepthOffset"];
}

//--------------------------------------------------------------
bool Effect::Load(const char* const effectFilename)
{
  bool loaded = false;

  const char* programName = std::strrchr(effectFilename, '.');
  if (!programName)
  {
    programName = effectFilename;
  }

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
    LOG("\ncould not parse %s::%s:\n%s\n", effectFilename, log);
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

      EffectUniform u;
      u.location = glGetUniformLocation(programHandle, uniformName);
      u.type = types[i];
      parameters[uniformName] = u;
		}
	}
}
