
struct Out
{
  float4 color : COLOR0;
  float4 normal : COLOR1;
};

Out main(uniform sampler2D colormap,
         uniform sampler2D normalmap,
         in float3 normal,
         in float2 mapping)
{
  float4 color = tex2D(colormap, mapping);

  clip(color.a - 0.5);

  Out result;
  result.color = color;
  result.normal = float4(normalize(normal) / 2 + 0.5, 0);
  return result;
}

