
float4 main(uniform sampler2D checkers, in float4 color, in float3 normal, in float2 mapping) : COLOR
{
  return tex2D(checkers, mapping) * 0.25 + color * dot(normal, float4(0, 0, 1, 0)) * 0.75;
}

