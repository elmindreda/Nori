
float4 main(uniform samplerCUBE cubemap, in float3 normal) : COLOR
{
  return texCUBE(cubemap, normalize(normal));
}

