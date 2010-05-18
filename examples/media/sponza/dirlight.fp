
struct Light
{
  float3 direction;
  float3 color;
};

float4 main(uniform samplerRECT colorbuffer,
            uniform samplerRECT normalbuffer,
            uniform Light light,
            in float2 mapping) : COLOR
{
  float3 color = texRECT(colorbuffer, mapping).rgb;
  float3 N = normalize(texRECT(normalbuffer, mapping).xyz - 0.5);

  float Ia = 0.2;
  float Id = saturate(dot(N, light.direction));

  return float4(color * light.color * (Ia + Id), 1);
}

