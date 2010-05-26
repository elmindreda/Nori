
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
  half minusPz = nearZ / (texRECT(depthbuffer, mapping).r * nearOverFarZminusOne + 1);
  half3 P = half3(clipOverF.x, clipOverF.y, -1) * minusPz;

  half dist = distance(P, light.position);
  if (dist > light.radius)
    return float4(0);

  half4 NS = texRECT(normalbuffer, mapping);
  half3 Cs = texRECT(colorbuffer, mapping).rgb;

  half3 L = normalize(light.position - P);
  half3 N = normalize(NS.xyz - 0.5);

  half Id = saturate(dot(N, L));
  half Is = 0.0;

  if (NS.a > 0.0)
  {
    half3 R = reflect(L, N);
    half3 E = normalize(P);

    Is = pow(saturate(dot(R, E)), 10) * NS.a;
  }

  half att = tex1D(light.distAttTexture, dist / light.radius).r;

  return float4(Cs * light.color * (Id + Is) * att, 1);
}

