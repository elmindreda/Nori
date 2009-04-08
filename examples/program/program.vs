
struct Out
{
  float4 position : POSITION;
  float4 color;
  float3 normal;
};

Out main(uniform float4x4 MVP, in float3 position, in float3 normal)
{
  Out result;

  result.position = mul(MVP, float4(position, 1));
  result.color = float4(1, 0, 0, 1);
  result.normal = normal;
  return result;
}

