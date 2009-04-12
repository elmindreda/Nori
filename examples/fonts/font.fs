
float4 main(uniform sampler2D glyphs, uniform float4 color, in float2 mapping) : COLOR
{
  return color * tex2D(glyphs, mapping).a;
}

