
float4 main(uniform sampler2D checkers, in float3 normal, in float2 mapping) : COLOR
{
  const float4 color = float4(1.0, 0.3, 0.3, 1);
  return color * (0.25 + tex2D(checkers, mapping).r * dot(normal, float4(0, 0, 0.75, 0)));
}

