
float4 main(uniform float4x4 P, in float2 position) : POSITION
{
  return mul(P, float4(position, 0, 1));
}

