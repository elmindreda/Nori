
float4 main(in float4 color, in float3 normal) : COLOR
{
  return color + float4(normal, 0);
}

