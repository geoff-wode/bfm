#include "common.glsl"

//---------------------------------------------------------

uniform float MaxHeight = 1000.0f;

//---------------------------------------------------------

struct VSOut
{
  vec4 clip;
};

//---------------------------------------------------------

float fBm(vec2 point, float maxRoughness, float lacunarity, float octaves)
{
  float value = 0;
  int i;
  for (i = 0; i < int(octaves); ++i)
  {
    value += noise1(point) * pow(lacunarity, -maxRoughness * i);
    point *= lacunarity;
  }
  float remainder = octaves - int(octaves);
  if (remainder > 0)
  {
    value += remainder * noise1(point) * pow(lacunarity, -maxRoughness * i);
  }
  return value;
}

//---------------------------------------------------------

// Vertex shader: computes clip-space position and forwards model-space coordinate to fragment shader.
shader VS
  (
    in vec2 Position : SHADER_SEMANTIC_POSITION,
    out VSOut vsOut
  )
{
  vec4 P = WorldMatrix * vec4(Position.x, 0.0f, Position.y, 1.0f);
  float height = MaxHeight * fBm(P.xz, 0.8f, 0.516273f, 6.3f);
  vec4 worldPos = vec4(P.x, height, P.z, 1.0f);
  gl_Position = ViewProjectionMatrix * worldPos;

  // Transform the vertex to clip space and output that to the fragment shader
  // so that it can compute depth logarithmically rather than using the built-in
  // fragment depth calculation.
  vsOut.clip = gl_Position;
}

//---------------------------------------------------------

shader FS
  (
    in VSOut vsIn,
    out vec4 colour
  )
{
  colour = vec4(1,1,1,1);

  gl_FragDepth = ComputeDepth(vsIn.clip.z);
}

//---------------------------------------------------------

program Terrain
{
  vs(420) = VS();
  fs(420) = FS();
};
