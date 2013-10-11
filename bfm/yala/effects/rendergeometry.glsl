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
uniform mat4 RTEWorldViewProjectionMatrix;

// World-unit distance to the far clip plane in metres, used when computed a logarithmic
// depth value.
uniform float FarClipPlaneDistance;

// Constant controlling the precision of the logarithmic depth buffer.
// Smaller values increase precision at a distance but reduce precision close in. Larger values
// has the obvious opposite effect.
uniform float LogDepthConstant = 0.001f;

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
  // Compute the position-relative-to-eye given double-precision vertex and camera positions
  // encoded as high and low single-precision values...
  vec3 highDifference = PositionHigh - CameraPositionHigh;
  vec3 lowDifference = PositionLow - CameraPositionLow;
  vec4 P = vec4(highDifference + lowDifference, 1);
  
  vec4 clippedPosition = RTEWorldViewProjectionMatrix * P;
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
  // Compute logarithmic depth instead of the default gl_FragCoord.z produced by GL...
  float upperLine = 2.0f * ((LogDepthConstant * vsIn.zClip) + 1.0f);
  float lowerLine = log((LogDepthConstant * FarClipPlaneDistance) + 1.0f);
  gl_FragDepth = (upperLine / lowerLine) - 1.0f;

  colour = vec4(0,1,0,1);
}

//---------------------------------------------------------

program RenderGeometry
{
  vs(420) = VS();
  fs(420) = FS();
};
