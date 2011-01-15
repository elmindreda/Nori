
float4 main(uniform sampler2D depthmap,
            uniform sampler2D spotmask,
            in float4 lightpos,
            in float3 lightvec,
            in float3 normal) : COLOR
{
  float depth = tex2Dproj(depthmap, lightpos).z;

  const float3 C = float3(1, 0.3, 0.3);
  const float3 Ia = float3(0.2);

  if (depth >= lightpos.z / lightpos.w)
  {
    float3 L = normalize(lightvec);
    float3 Id = saturate(dot(normal, L)) * tex2Dproj(spotmask, lightpos).r;

    return float4(C * (Ia + Id), 1);
  }
  else
    return float4(C * Ia, 1);
}

