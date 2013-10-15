#if ! defined(__TERRAIN_EFFECT__)
#define __TERRAIN_EFFECT__

#include <effect.h>
#include <effectuniform.h>

// Class representing an effect which renders terrain "relative-to-eye" space,
// allowing for massive objects and view distances.
class TerrainEffect : public Effect
{
public:
  TerrainEffect() { }
  virtual ~TerrainEffect() { }

  // The world-view-projection matrix.
  // This must be computed by creating a model-view matrix and setting its translation
  // vector to (0,0,0) _before_ the projection matrix is applied. E.g.
  //
  //    glm::dmat4 modelViewMatrix = cameraviewMatrix * worldTransform;
  //    modelViewMatrix[3][0] = 0.0;
  //    modelViewMatrix[3][1] = 0.0;
  //    modelViewMatrix[3][2] = 0.0;
  //    glm::mat4 rteWVP = glm::mat4(context.camera->projectionMatrix * modelViewMatrix));
  //
  // Also note that the final rteWVP matrix must be cast to single-precision before passing
  // it to the effect.
  EffectUniform* WorldViewProjectionMatrix;

  // In order to use relative-to-eye rendering, the camera's world-space position
  // must be passed in as a double-precision value (encoded into these 2 single-
  // precision uniforms).
  EffectUniform* CameraPositionLow;
  EffectUniform* CameraPositionHigh;

  // Values with the prefix "LogDepth" control the logarithmic depth buffer.
  // Logarithmic depth is computed as
  //
  //  | 2 * [ ( C * zClip) + 1] |
  //  | ----------------------- | - 1
  //  |    log(C * fClip) + 1   |
  //
  // where,
  // - C is a constant controlling depth precision (smaller increases precision at
  //    distance at the expense of loosing precision close-in.
  // - zClip is the clip-space (ie. post-projection transform) z coordinate
  // - fClip is the distance to the far clip plane (in world units).
  // Note that the divisor only changes when the far clip distance changes, so this
  // should be computed outside of the vertex shader.
  EffectUniform* LogDepthConstant;
  EffectUniform* LogDepthDivisor;

private:
  virtual void Initialise()
  {
    WorldViewProjectionMatrix = &parameters["WorldViewProjectionMatrix"];
    CameraPositionLow = &parameters["CameraPositionLow"];
    CameraPositionHigh = &parameters["CameraPositionHigh"];
    LogDepthDivisor = &parameters["LogDepthDivisor"];
    LogDepthConstant = &parameters["LogDepthConstant"];
  }
};

#endif // __TERRAIN_EFFECT__
