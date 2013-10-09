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

//---------------------------------------------------------

shader VS
  (
    in vec3 PositionLow : SHADER_SEMANTIC_POSITION_LOW,
    in vec3 PositionHigh: SHADER_SEMANTIC_POSITION_HIGH
  )
{
  // Compute the position-relative-to-eye given double-precision vertex and camera positions
  // encoded as high and low single-precision values...
  vec3 highDifference = PositionHigh - CameraPositionHigh;
  vec3 lowDifference = PositionLow - CameraPositionLow;
  vec4 P = vec4(highDifference + lowDifference, 1);
  gl_Position = RTEWorldViewProjectionMatrix * P;
}

//---------------------------------------------------------

shader FS
  (
    out vec4 colour
  )
{
  colour = vec4(0,1,0,1);
}

//---------------------------------------------------------

program RenderGeometry
{
  vs(420) = VS();
  fs(420) = FS();
};
