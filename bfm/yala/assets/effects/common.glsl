// Common definitions, uniforms and functions.

//---------------------------------------------------------


// World space camera position.
uniform vec3 CameraPosition;

// Transforms.
uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewProjectionMatrix;
uniform mat4 WorldViewProjectionMatrix;

// Per-frame constants controlling the computation of logarithmic depth.
uniform float LogDepthConstant;
uniform float LogDepthOffset;
uniform float LogDepthDivisor;

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
