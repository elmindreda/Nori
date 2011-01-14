
float4 main(uniform sampler2D glyphs, uniform float4 color, in float2 mapping) : COLOR
{
  return float4(color.rgb, color.a * tex2D(glyphs, mapping).r);
}

