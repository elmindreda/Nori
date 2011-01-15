
const float3 L = normalize(float3(1, 0, 1));

float4 main(in float3 viewpos, in float3 normal) : COLOR
{
  float3 R = reflect(L, normal);
  float3 V = viewpos;

  float Is = pow(saturate(dot(R, V)), 15);

  return float4(float3(1) * Is, 1);
}

