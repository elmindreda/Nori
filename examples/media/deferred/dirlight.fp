
struct Light
{
  float3 direction;
  float3 color;
};

float4 main(uniform samplerRECT colorTexture,
            uniform samplerRECT normalTexture,
            uniform samplerRECT depthTexture,
            uniform float nearZ,
            uniform float nearOverFarZminusOne,
            uniform Light light,
            in float2 mapping,
            in float2 clipOverF) : COLOR
{
  half4 NS = texRECT(normalTexture, mapping);
  half3 Cs = texRECT(colorTexture, mapping).rgb;
  half minusPz = nearZ / (texRECT(depthTexture, mapping).r * nearOverFarZminusOne + 1);

  half3 P = half3(clipOverF.x, clipOverF.y, -1) * minusPz;
  half3 N = normalize(NS.xyz - 0.5);
  half3 L = light.direction;

  half Id = saturate(dot(N, light.direction));
  half Is = 0.0;

  if (NS.a > 0.0)
  {
    half3 R = reflect(L, N);
    half3 E = normalize(P);

    Is = pow(saturate(dot(R, E)), 10) * NS.a;
  }

  return float4(Cs * light.color * (Id + Ia), 1);
}

