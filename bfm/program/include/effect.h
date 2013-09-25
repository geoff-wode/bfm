#if ! defined(__EFFECT__)
#define __EFFECT__

#include <SDL.h>
#include <logging.h>
#include <glfx.h>
#include <map>
#include <vector>
#include <string>
#include <gl_loader/gl_loader.h>
#include <boost/foreach.hpp>

#define SHADER_SEMANTIC_POSITION  0
#define SHADER_SEMANTIC_NORMAL    1
#define SHADER_SEMANTIC_TEXCOORD  2

struct Effect
{
  GLuint handle;
  std::map<std::string, GLint> params;

  Effect() { }
  ~Effect() { glDeleteProgram(handle); }

  void Load(const char* const file, const char* const programName)
  {
    const int glfx = glfxGenEffect();
    if (glfxParseEffectFromFile(glfx, file))
    {
      handle = glfxCompileProgram(glfx, programName);
      if (-1 != handle)
      {
        LoadEffectParams(handle, params);
        LOG("effect %s::%s compiled - uniform params:\n", file, programName);
        BOOST_FOREACH(auto v, params)
        {
          LOG("  %s\n", v.first.c_str());
        }
      }
      else
      {
        LOG("\n%s::%s\n%s\n", file, programName, glfxGetEffectLog(glfx));
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, programName, glfxGetEffectLog(glfx), NULL);
        exit(EXIT_FAILURE);
      }
    }
    else
    {
      LOG("\ncould not parse %s::%s:\n%s\n", file, programName, glfxGetEffectLog(glfx));
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, programName, glfxGetEffectLog(glfx), NULL);
      exit(EXIT_FAILURE);
    }

    glfxDeleteEffect(glfx);
  }

private:
  static void LoadEffectParams(GLuint effect, std::map<std::string,GLint>& params)
  {
	  GLint numUniforms;
	  glGetProgramiv(effect, GL_ACTIVE_UNIFORMS, &numUniforms);

	  std::vector<GLuint> indices(numUniforms);
	  for (GLint i = 0; i < numUniforms; ++i) { indices[i] = i; }

	  std::vector<GLint> types(numUniforms);
	  glGetActiveUniformsiv(effect, numUniforms, indices.data(), GL_UNIFORM_TYPE, types.data());

	  std::vector<GLint> blockIndices(numUniforms);
	  glGetActiveUniformsiv(effect, numUniforms, indices.data(), GL_UNIFORM_BLOCK_INDEX, blockIndices.data());

	  for (int i = 0; i < numUniforms; ++i)
	  {
		  if (-1 == blockIndices[i])
		  {
			  // not a named uniform block...
			  char uniformName[256] = { 0 };
			  glGetActiveUniformName(effect, i, sizeof(uniformName)-1, NULL, uniformName);
			  params[uniformName] = glGetUniformLocation(effect, uniformName);
		  }
	  }
  }

};

#endif // __EFFECT__
