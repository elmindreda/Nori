
float4 main(in float4 color, in float3 normal, in float2 mapping) : COLOR
{
  return color + float4(normal, 0) + float4(mapping, 0, 0);
}

