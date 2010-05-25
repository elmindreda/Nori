
struct Light
{
  float3 position;
  float3 color;
  float radius;
  sampler1D distAttTexture;
};

float4 main(uniform samplerRECT colorbuffer,
            uniform samplerRECT normalbuffer,
            uniform samplerRECT depthbuffer,
            uniform float nearZ,
            uniform float nearOverFarZminusOne,
            uniform Light light,
            in float2 mapping,
            in float2 clipOverF) : COLOR
{
  float minusPz = nearZ / (texRECT(depthbuffer, mapping).r * nearOverFarZminusOne + 1);
  float3 P = float3(clipOverF.x, clipOverF.y, -1) * minusPz;

  float dist = distance(P, light.position);
  if (dist > light.radius)
    return float4(0);

  float4 NS = texRECT(normalbuffer, mapping);
  float3 Cs = texRECT(colorbuffer, mapping).rgb;

  float3 L = normalize(light.position - P);
  float3 N = normalize(NS.xyz - 0.5);

  float Id = saturate(dot(N, L));
  float Is = 0.0;

  if (NS.a > 0.0)
  {
    float3 R = reflect(L, N);
    float3 E = normalize(P);

    Is = pow(saturate(dot(R, E)), 10) * NS.a;
  }

  float att = tex1D(light.distAttTexture, dist / light.radius).r;

  return float4(Cs * light.color * (Id + Is) * att, 1);
}

