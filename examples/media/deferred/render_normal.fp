
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
  float4 NS = tex2D(normalmap, mapping);

  Out result;
  result.color = tex2D(colormap, mapping);
  result.normal = float4(normalize(normal) / 2 + 0.5, NS.a);
  return result;
}

