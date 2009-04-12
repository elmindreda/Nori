
float4 main(uniform sampler2D image, in float2 mapping) : COLOR
{
  return tex2D(image, mapping);
}

