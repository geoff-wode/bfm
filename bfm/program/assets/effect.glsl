#define SHADER_SEMANTIC_POSITION  0
#define SHADER_SEMANTIC_NORMAL    1
#define SHADER_SEMANTIC_TEXCOORD  2

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewProjectionMatrix;
uniform mat4 WorldViewProjectionMatrix;
uniform mat3 NormalMatrix;

//-----------------------------------------------------------------------

struct VSOutput
{
  vec3 objPos;
  vec3 worldPos;
  vec3 objNormal;
  vec3 worldNormal;
  vec2 texcoord;
};

shader VSpassthrutransform(
  in vec3 Position : SHADER_SEMANTIC_POSITION,
  in vec3 Normal   : SHADER_SEMANTIC_NORMAL,
  in vec2 TexCoord : SHADER_SEMANTIC_TEXCOORD,
  out VSOutput vsOut)
{
  vec4 P = WorldViewProjectionMatrix * vec4(Position, 1);

  gl_Position = P;

  vsOut.objPos = Position;
  vsOut.worldPos = P.xyz;
  vsOut.objNormal = Normal;
  vsOut.worldNormal = NormalMatrix * Normal;
  vsOut.texcoord = TexCoord;
}

//-----------------------------------------------------------------------

uniform float PlanetRadius;
uniform sampler2D HeightMapSampler;

shader VSdisplacementMap(
  in vec3 Position : SHADER_SEMANTIC_POSITION,
  out VSOutput vsOut)
{
  vec4 P = vec4(normalize(Position) * PlanetRadius, 1);
  P.y += texture2D(HeightMapSampler, vec2(Position.x, Position.z)).r;

  gl_Position = WorldViewProjectionMatrix * P;
}

//-----------------------------------------------------------------------
shader FSgreen(in VSOutput vsIn, out vec4 colour)
{
  colour = vec4(0,1,0,1);
}

//-----------------------------------------------------------------------
program Green
{
  vs(420) = VSpassthrutransform();
  fs(420) = FSgreen();
};

program GreenDisplacement
{
  vs(420) = VSdisplacementMap();
  fs(420) = FSgreen();
};
