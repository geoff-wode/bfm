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

// Per-frame constants controlling the computation of logarithmic depth.
uniform float LogDepthConstant;
uniform float LogDepthOffset;
uniform float LogDepthDivisor;

//---------------------------------------------------------

struct VSOut
{
  vec4 clippedPos;
};

//---------------------------------------------------------

//#define USE_GPU_RTE
#define USE_GPU_RTE_DSFUN90

shader VS
  (
    in vec3 PositionLow : SHADER_SEMANTIC_POSITION_LOW,
    in vec3 PositionHigh: SHADER_SEMANTIC_POSITION_HIGH,
    out VSOut vsOut
  )
{
#if defined(USE_GPU_RTE)

  // Compute the model position in eye space given double-precision vertex and camera positions
  // encoded as high and low single-precision values...
  const vec3 highDifference = PositionHigh - CameraPositionHigh;
  const vec3 lowDifference = PositionLow - CameraPositionLow;
  
#elif defined(USE_GPU_RTE_DSFUN90)

  // Use "DSFUN90" to compute the model position in eye space given double-precision vertex and camera positions
  // encoded as high and low single-precision values...
  const vec3 t1 = PositionLow - CameraPositionLow;
  const vec3 e = t1 - PositionLow;
  const vec3 t2 = ((-CameraPositionLow - e) + (PositionLow - (t1 - e))) + PositionHigh - CameraPositionHigh;
  const vec3 highDifference = t1 + t2;
  const vec3 lowDifference = t2 - (highDifference - t1);

#endif

  // The model position, relative-to-eye...
  const vec4 rtePos = vec4(highDifference + lowDifference, 1);

  // Transform, project and clip...
  vsOut.clippedPos = WorldViewProjectionMatrix * rtePos;
  gl_Position = vsOut.clippedPos;
}

//---------------------------------------------------------

shader FS
  (
    in VSOut vsIn,
    out vec4 colour
  )
{
  // Compute logarithmic depth instead of the default gl_FragCoord.z emitted by GL.
  // See http://www.gamedev.net/blog/73/entry-2006307-tip-of-the-day-logarithmic-zbuffer-artifacts-fix
  // but basically:
  //
  //          | log((C * zClip) + offset) |
  //  depth = | ------------------------- |
  //          |  log((C * far) + offset)  |
  //
  // where,
  // - C is a constant controlling depth precision (smaller increases precision at
  //    distance at the expense of loosing precision close-in.
  // - zClip is the clip-space (ie. post-projection transform) z coordinate
  // - offset controls how close the near clip plane can be to the camera (bigger == less near clipping)
  //    (e.g. 2.0 gives a clip plane of a few centimeters)
  // - far is the distance from the camera to the far clip plane (in world units).
  // Since the divisor is constant for a whole render frame, "1/log((C * far) + near)" is
  // precomputed on the CPU.
  gl_FragDepth = log((LogDepthConstant * vsIn.clippedPos.z) + LogDepthOffset) * LogDepthDivisor;

  colour = vec4(0,1,0,1);
}

//---------------------------------------------------------

program Terrain
{
  vs(420) = VS();
  fs(420) = FS();
};
