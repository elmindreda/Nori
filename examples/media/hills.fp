
const float3 L = normalize(float3(1, 0, 0));

float4 main(uniform sampler2D depthmap, uniform sampler2D spotmask, uniform float3 light, in float4 lightpos, in float3 worldpos, in float3 normal) : COLOR
{
  float depth = tex2Dproj(depthmap, lightpos).z;

  float3 L = normalize(light - worldpos);

  if (depth >= lightpos.z / lightpos.w)
    return float4(float3(0.3 + saturate(dot(normal, L)) * tex2Dproj(spotmask, lightpos)), 1);
  else
    return float4(float3(0.3), 1);
}

