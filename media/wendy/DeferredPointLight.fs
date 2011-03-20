
struct Light
{
  vec3 position;
  vec3 color;
  float radius;
  sampler1D distAttTexture;
};

uniform sampler2DRect colorTexture;
uniform sampler2DRect normalTexture;
uniform sampler2DRect depthTexture;
uniform float nearZ;
uniform float nearOverFarZminusOne;
uniform Light light;

varying vec2 texCoord;
varying vec2 clipOverF;

void main()
{
  float minusPz = nearZ / (texture2DRect(depthTexture, texCoord).r * nearOverFarZminusOne + 1.0);
  vec3 P = vec3(clipOverF.x, clipOverF.y, -1.0) * minusPz;

  float dist = distance(P, light.position);
  if (dist > light.radius)
  {
    gl_FragColor = vec4(0.0);
    return;
  }

  vec4 NS = texture2DRect(normalTexture, texCoord);
  vec3 Cs = texture2DRect(colorTexture, texCoord).rgb;

  vec3 L = normalize(light.position - P);
  vec3 N = normalize(NS.xyz - 0.5);

  float Id = clamp(dot(N, L), 0.0, 1.0);
  float Is = 0.0;

  if (NS.a > 0.0)
  {
    vec3 R = reflect(L, N);
    vec3 E = normalize(P);

    Is = pow(clamp(dot(R, E), 0.0, 1.0), 10.0) * NS.a;
  }

  float att = texture1D(light.distAttTexture, dist / light.radius).r;

  gl_FragColor = vec4(Cs * light.color * (Id + Is) * att, 1.0);
}

