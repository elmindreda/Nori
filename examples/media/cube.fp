
const float3 L = normalize(float3(1, 0, 1));

float4 main(uniform sampler2D checkers, in float3 viewpos, in float3 normal, in float2 mapping) : COLOR
{
  float4 color = float4(1.0, 0.3, 0.3, 1) * tex2D(checkers, mapping).r;

  float3 I = normalize(viewpos);

  float3 ambient = float3(0.1, 0.1, 0.1);
  float3 diffuse = dot(normal, L);
  float3 specular = dot(reflect(I, normal), L);

  return color * (float4(ambient, 1) + float4(diffuse, 1) + float4(specular, 1));
}

