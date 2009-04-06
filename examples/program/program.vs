
struct Out
{
  float4 position : POSITION;
  float4 color;
  float3 normal;
  float2 mapping;
};

Out main(in float3 position, in float3 normal, in float2 mapping)
{
  Out result;

  result.position = float4(position, 1);
  result.color = float4(1, 0, 0, 1);
  result.normal = normal;
  result.mapping = mapping;
  return result;
}

