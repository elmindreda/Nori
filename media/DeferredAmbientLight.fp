
struct Light
{
  float3 color;
};

float4 main(uniform samplerRECT colorTexture,
            uniform Light light,
            in float2 mapping,
            in float2 clipOverF) : COLOR
{
  half3 Cs = texRECT(colorTexture, mapping).rgb;

  return float4(Cs * light.color, 1);
}

