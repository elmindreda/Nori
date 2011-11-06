#extension GL_ARB_texture_rectangle : require

struct Light
{
  vec3 color;
};

uniform Light light;
uniform sampler2DRect colorTexture;

varying vec2 texCoord;
varying vec2 clipOverF;

void main()
{
  vec3 Cs = texture2DRect(colorTexture, texCoord).rgb;

  gl_FragColor = vec4(Cs * light.color, 1);
}

