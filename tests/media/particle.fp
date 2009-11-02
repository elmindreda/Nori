
float4 main(uniform sampler2D image, in float4 color, in float2 mapping) : COLOR
{
  return float4(float3(color.r) * tex2D(image, mapping).rgb, 1);
}

