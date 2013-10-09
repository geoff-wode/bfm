#define SHADER_SEMANTIC_POSITION  0
#define SHADER_SEMANTIC_NORMAL    1
#define SHADER_SEMANTIC_TEXCOORD  2

//---------------------------------------------------------

uniform mat4 WorldViewProjectionMatrix;

//---------------------------------------------------------

shader VS
  (
    in vec3 Position : SHADER_SEMANTIC_POSITION
  )
{
  gl_Position = WorldViewProjectionMatrix * vec4(Position, 1);
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
