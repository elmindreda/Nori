
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
  float3 normal = normalize((texRECT(normalbuffer, mapping).xyz - float3(0.5)) * 2);

  return float4(color * light.color * saturate(dot(normal, light.direction)), 1);
}

