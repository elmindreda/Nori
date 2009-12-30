
const float3 L = normalize(float3(1, 0, 1));

float4 main(uniform sampler2D depthmap, in float4 lightpos, in float3 normal) : COLOR
{
  float depth = tex2Dproj(depthmap, lightpos).z;

  if (depth >= lightpos.z / lightpos.w)
    return float4(float3(0.3 + dot(normal, float3(0, 0, 1))), 1);
  else
    return float4(float3(0.3), 1);
}

