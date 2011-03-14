
struct Light
{
  vec3 direction;
  vec3 color;
};

uniform sampler2DRect colorTexture;
uniform sampler2DRect normalTexture;
uniform sampler2DRect depthTexture;
uniform float nearZ;
uniform float nearOverFarZminusOne;
uniform Light light;

varying vec2 mapping;
varying vec2 clipOverF;

void main()
{
  vec4 NS = texture2DRect(normalTexture, mapping);
  vec3 Cs = texture2DRect(colorTexture, mapping).rgb;
  float minusPz = nearZ / (texture2DRect(depthTexture, mapping).r * nearOverFarZminusOne + 1);

  vec3 P = vec3(clipOverF.x, clipOverF.y, -1) * minusPz;
  vec3 N = normalize(NS.xyz - 0.5);
  vec3 L = light.direction;

  float Id = clamp(dot(N, light.direction), 0, 1);
  float Is = 0.0;

  if (NS.a > 0.0)
  {
    vec3 R = reflect(L, N);
    vec3 E = normalize(P);

    Is = pow(clamp(dot(R, E), 0, 1), 10) * NS.a;
  }

  return vec4(Cs * light.color * (Id + Is), 1);
}

