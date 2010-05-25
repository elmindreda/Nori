
struct Light
{
  float3 position;
  float3 color;
  float linear;
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
  float4 NS = texRECT(normalbuffer, mapping);
  float3 Cs = texRECT(colorbuffer, mapping).rgb;
  float minusPz = nearZ / (texRECT(depthbuffer, mapping).r * nearOverFarZminusOne + 1);

  float3 P = float3(clipOverF.x * minusPz, clipOverF.y * minusPz, -minusPz);
  float3 L = normalize(light.position - P);
  float3 N = normalize(NS.xyz - 0.5);
  float3 R = reflect(L, N);
  float3 E = normalize(P);

  float Ia = 0.2;
  float Id = saturate(dot(N, L));
  float Is = saturate(dot(R, E));

  float att = 1.0 / (distance(P, light.position) * light.linear);

  return float4(Cs * light.color * (Ia + Id + NS.a * pow(Is, 10)) * att, 1);
}

