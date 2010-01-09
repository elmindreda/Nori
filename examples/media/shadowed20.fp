
float4 main(uniform sampler2D depthmap,
            in float4 lightpos,
            in float3 normal) : COLOR
{
  float depth = tex2D(depthmap, lightpos.xy / lightpos.w).z;

  const float3 C = float3(1, 0.3, 0.3);
  const float3 Ia = float3(0.2);

  if (depth >= lightpos.z / lightpos.w)
  {
    float L = normalize(float3(1, 0, 0));
    float Id = saturate(dot(normal, L));

    return float4(C * (Ia + Id), 1);
  }
  else
    return float4(C * Ia, 1);
}

