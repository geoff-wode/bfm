#define SHADER_SEMANTIC_POSITION_LOW  0
#define SHADER_SEMANTIC_POSITION_HIGH 1
#define SHADER_SEMANTIC_NORMAL        2
#define SHADER_SEMANTIC_TEXCOORD      3

//---------------------------------------------------------

// Double-precision world space camera position, encoded into 2 single-precision values..
uniform vec3 CameraPositionLow;
uniform vec3 CameraPositionHigh;

// This matrix holds the relative-to-eye MVP matrix (ie. it has had it's translation
// set to (0,0,0) before the projection transform is applied)...
uniform mat4 WorldViewProjectionMatrix;

// Constant controlling the precision of the logarithmic depth buffer.
// Smaller values increase precision at a distance but reduce precision close in. Larger values
// have the obvious opposite effect.
uniform float LogDepthConstant;

// log(LogDepthConstant * fClip) + 1
uniform float LogDepthDivisor;

//---------------------------------------------------------

struct VSOut
{
  float zClip;
};

//---------------------------------------------------------

shader VS
  (
    in vec3 PositionLow : SHADER_SEMANTIC_POSITION_LOW,
    in vec3 PositionHigh: SHADER_SEMANTIC_POSITION_HIGH,
    out VSOut vsOut
  )
{
  // Compute the model position in eye space given double-precision vertex and camera positions
  // encoded as high and low single-precision values...
  const vec3 highDifference = PositionHigh - CameraPositionHigh;
  const vec3 lowDifference = PositionLow - CameraPositionLow;
  // The model position, relative-to-eye...
  const vec4 rtePos = vec4(highDifference + lowDifference, 1);
  
  const vec4 clippedPosition = WorldViewProjectionMatrix * rtePos;
  gl_Position = clippedPosition;

  vsOut.zClip = clippedPosition.z;
}

//---------------------------------------------------------

shader FS
  (
    in VSOut vsIn,
    out vec4 colour
  )
{
  // Compute logarithmic depth instead of the default gl_FragCoord.z emitted by GL:
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
  // Since the divisor is constant for a whole render frame, it is precomputed on the CPU.
  float upperLine = 2.0f * ((LogDepthConstant * vsIn.zClip) + 1.0f);
  gl_FragDepth = (upperLine / LogDepthDivisor) - 1.0f;

  colour = vec4(0,1,0,1);
}

//---------------------------------------------------------

program Terrain
{
  vs(420) = VS();
  fs(420) = FS();
};
