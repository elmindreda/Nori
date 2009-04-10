
struct Out
{
  float4 position : POSITION;
  float4 color;
  float3 normal;
  float2 mapping;
};

Out main(uniform float4x4 MVP, uniform float4x4 MV, in float3 position, in float3 normal)
{
  Out result;

  float3 eyepos = mul(MV, float4(position, 1)).xyz;

  float3 u = normalize(eyepos);
  float3 n = mul(MV, float4(normal, 0));
  float3 r = reflect(u, n);
  float3 t = r + float3(0, 0, 1);
  float p = sqrt(dot(t, t));

  result.position = mul(MVP, float4(position, 1));
  result.color = float4(1, 0, 0, 1);
  result.normal = n;
  result.mapping = float2(0.5 + r.x / (2 * p), 0.5 + r.y / (2 * p));
  return result;
}

