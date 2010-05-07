
struct Light
{
  float3 direction;
  float3 color;
};

float4 main(uniform samplerRECT colorbuffer,
            uniform samplerRECT normalbuffer,
            /*
            uniform float minZ,
            uniform float maxZ,
            uniform float halfFOV,
            uniform float aspect,
            in Light light,
            */
            in float2 mapping) : COLOR
{
  float3 color = texRECT(colorbuffer, mapping).rgb;
  float3 normal = texRECT(normalbuffer, mapping).xyz;

  //return float4(color * light.color * dot(normal, light.direction), 1);
  return float4(color * dot(normal, float3(1, 0, 0)), 1);
}

