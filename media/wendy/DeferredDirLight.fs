#extension GL_ARB_texture_rectangle : require

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

varying vec2 texCoord;
varying vec2 clipOverF;

void main()
{
  vec4 NS = texture2DRect(normalTexture, texCoord);
  vec3 Cs = texture2DRect(colorTexture, texCoord).rgb;
  float minusPz = nearZ / (texture2DRect(depthTexture, texCoord).r * nearOverFarZminusOne + 1.0);

  vec3 P = vec3(clipOverF.x, clipOverF.y, -1.0) * minusPz;
  vec3 N = normalize(NS.xyz - 0.5);
  vec3 L = light.direction;

  float Id = clamp(dot(N, light.direction), 0.0, 1.0);
  float Is = 0.0;

  if (NS.a > 0.0)
  {
    vec3 R = reflect(L, N);
    vec3 E = normalize(P);

    Is = pow(clamp(dot(R, E), 0.0, 1.0), 10.0) * NS.a;
  }

  gl_FragColor = vec4(Cs * light.color * (Id + Is), 1.0);
}

