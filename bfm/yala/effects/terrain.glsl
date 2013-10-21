#define SHADER_SEMANTIC_POSITION      0
#define SHADER_SEMANTIC_NORMAL        2
#define SHADER_SEMANTIC_TEXCOORD      3

// World space camera position.
uniform vec3 CameraPosition;

// World-View-Projection transform.
uniform mat4 WorldViewProjectionMatrix;

// Per-frame constants controlling the computation of logarithmic depth.
uniform float LogDepthConstant;
uniform float LogDepthOffset;
uniform float LogDepthDivisor;

uniform vec3 Centre;
uniform float Radius;
uniform float Width;

//---------------------------------------------------------

struct VSOut
{
  vec4 clippedPos;  // The fully transformed clip-space position.
};

//---------------------------------------------------------
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
float ComputeDepth(float Zclip)
{
  return log((LogDepthConstant * Zclip) + LogDepthOffset) * LogDepthDivisor;
}

//---------------------------------------------------------

// Vertex shader: computes clip-space position and forwards model-space coordinate to fragment shader.
shader VS
  (
    in vec3 Position : SHADER_SEMANTIC_POSITION,
    out VSOut vsOut
  )
{
  //const vec3 p = Radius * normalize((Width * Position)) + Centre);
  vsOut.clippedPos = WorldViewProjectionMatrix * vec4(Position, 1.0);

  gl_Position = vsOut.clippedPos;
}

//---------------------------------------------------------

shader FS
  (
    in VSOut vsIn,
    out vec4 colour
  )
{
  colour = vec4(0,1,0,1);
  gl_FragDepth = ComputeDepth(vsIn.clippedPos.z);
}

//---------------------------------------------------------

program Terrain
{
  vs(420) = VS();
  fs(420) = FS();
};
