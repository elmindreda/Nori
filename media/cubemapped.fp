
float4 main(uniform samplerCUBE cubemap, in float3 eyepos, in float3 normal) : COLOR
{
  float3 R = reflect(-eyepos, normal);
  return texCUBE(cubemap, R);
}

