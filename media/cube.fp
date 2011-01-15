
const float3 L = normalize(float3(1, 0, 1));

float4 main(uniform sampler2D checkers, in float3 viewpos, in float3 normal, in float2 mapping) : COLOR
{
  float3 C = float3(1.0, 0.3, 0.3) * tex2D(checkers, mapping).r;

  float3 R = reflect(L, normal);
  float3 V = viewpos;

  float Ia = 0.1;
  float Id = saturate(dot(normal, L));
  float Is = pow(saturate(dot(R, V)), 15);

  return float4(C * (Ia + Id) + float3(1) * Is, 1);
}

