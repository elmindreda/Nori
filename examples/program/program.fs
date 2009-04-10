
float4 main(uniform sampler2D checkers, in float4 color, in float3 normal, in float2 mapping) : COLOR
{
  return float4(tex2D(checkers, mapping).r) * 0.25 + color * dot(normal, float4(0, 0, 1, 0)) * 0.75;
}

