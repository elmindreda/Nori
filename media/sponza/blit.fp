
float4 main(uniform samplerRECT image, in float2 mapping) : COLOR
{
  return texRECT(image, mapping);
}

