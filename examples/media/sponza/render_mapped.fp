
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
  Out result;
  result.color = tex2D(colormap, mapping);
  result.normal = float4(normalize(normal) / 2 + float3(0.5), 0);
  return result;
}

